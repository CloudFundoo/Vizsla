/** Initial work is from mongoose project **/

#ifndef VIZSLA_HEADER_INCLUDED
#define  VIZSLA_HEADER_INCLUDED

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct vz_context;     // Handle for the HTTP service itself
struct vz_connection;  // Handle for the individual connection


// This structure contains information about the HTTP request.
struct vz_request_info {
  void *user_data;       // User-defined pointer passed to vz_start()
  char *request_method;  // "GET", "POST", etc
  char *uri;             // URL-decoded URI
  char *http_version;    // E.g. "1.0", "1.1"
  char *query_string;    // URL part after '?' (not including '?') or NULL
  char *remote_user;     // Authenticated user, or NULL if no auth used
  char *log_message;     // Vizsla error log message, MG_EVENT_LOG only
  long remote_ip;        // Client's IP address
  int remote_port;       // Client's port
  int status_code;       // HTTP reply status code, e.g. 200
  int is_ssl;            // 1 if SSL-ed, 0 if not
  int num_headers;       // Number of headers
  struct vz_header {
    char *name;          // HTTP header name
    char *value;         // HTTP header value
  } http_headers[64];    // Maximum 64 headers
};

// Various events on which user-defined function is called by Vizsla.
enum vz_event {
  MG_NEW_REQUEST,   // New HTTP request has arrived from the client
  MG_HTTP_ERROR,    // HTTP error must be returned to the client
  MG_EVENT_LOG,     // Vizsla logs an event, request_info.log_message
  MG_INIT_SSL,      // Vizsla initializes SSL. Instead of vz_connection *,
                    // SSL context is passed to the callback function.
  MG_REQUEST_COMPLETE  // Vizsla has finished handling the request
};

// Prototype for the user-defined function. Vizsla calls this function
// on every MG_* event.
//
// Parameters:
//   event: which event has been triggered.
//   conn: opaque connection handler. Could be used to read, write data to the
//         client, etc. See functions below that have "vz_connection *" arg.
//   request_info: Information about HTTP request.
//
// Return:
//   If handler returns non-NULL, that means that handler has processed the
//   request by sending appropriate HTTP reply to the client. Vizsla treats
//   the request as served.
//   If handler returns NULL, that means that handler has not processed
//   the request. Handler must not send any data to the client in this case.
//   Vizsla proceeds with request handling as if nothing happened.
typedef void * (*vz_callback_t)(enum vz_event event,
                                struct vz_connection *conn,
                                const struct vz_request_info *request_info);


// Start web server.
//
// Parameters:
//   callback: user defined event handling function or NULL.
//   options: NULL terminated list of option_name, option_value pairs that
//            specify Vizsla configuration parameters.
//
// Side-effects: on UNIX, ignores SIGCHLD and SIGPIPE signals. If custom
//    processing is required for these, signal handlers must be set up
//    after calling vz_start().
//
//
// Example:
//   const char *options[] = {
//     "document_root", "/var/www",
//     "listening_ports", "80,443s",
//     NULL
//   };
//   struct vz_context *ctx = vz_start(&my_func, NULL, options);
//
//
// Return:
//   web server context, or NULL on error.
struct vz_context *vz_start(vz_callback_t callback, void *user_data,
                            const char **options);


// Stop the web server.
//
// Must be called last, when an application wants to stop the web server and
// release all associated resources. This function blocks until all Vizsla
// threads are stopped. Context pointer becomes invalid.
void vz_stop(struct vz_context *);


// Get the value of particular configuration parameter.
// The value returned is read-only. Vizsla does not allow changing
// configuration at run time.
// If given parameter name is not valid, NULL is returned. For valid
// names, return value is guaranteed to be non-NULL. If parameter is not
// set, zero-length string is returned.
const char *vz_get_option(const struct vz_context *ctx, const char *name);


// Return array of strings that represent valid configuration options.
// For each option, a short name, long name, and default value is returned.
// Array is NULL terminated.
const char **vz_get_valid_option_names(void);


// Add, edit or delete the entry in the passwords file.
//
// This function allows an application to manipulate .htpasswd files on the
// fly by adding, deleting and changing user records. This is one of the
// several ways of implementing authentication on the server side. For another,
// cookie-based way please refer to the examples/chat.c in the source tree.
//
// If password is not NULL, entry is added (or modified if already exists).
// If password is NULL, entry is deleted.
//
// Return:
//   1 on success, 0 on error.
int vz_modify_passwords_file(const char *passwords_file_name,
                             const char *domain,
                             const char *user,
                             const char *password);

// Send data to the client.
int vz_write(struct vz_connection *, const void *buf, size_t len);


// Send data to the browser using printf() semantics.
//
// Works exactly like vz_write(), but allows to do message formatting.
// Note that vz_printf() uses internal buffer of size IO_BUF_SIZE
// (8 Kb by default) as temporary message storage for formatting. Do not
// print data that is bigger than that, otherwise it will be truncated.
int vz_printf(struct vz_connection *, const char *fmt, ...)
#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
;


// Send contents of the entire file together with HTTP headers.
void vz_send_file(struct vz_connection *conn, const char *path);


// Read data from the remote end, return number of bytes read.
int vz_read(struct vz_connection *, void *buf, size_t len);


// Get the value of particular HTTP header.
//
// This is a helper function. It traverses request_info->http_headers array,
// and if the header is present in the array, returns its value. If it is
// not present, NULL is returned.
const char *vz_get_header(const struct vz_connection *, const char *name);


// Get a value of particular form variable.
//
// Parameters:
//   data: pointer to form-uri-encoded buffer. This could be either POST data,
//         or request_info.query_string.
//   data_len: length of the encoded data.
//   var_name: variable name to decode from the buffer
//   buf: destination buffer for the decoded variable
//   buf_len: length of the destination buffer
//
// Return:
//   On success, length of the decoded variable.
//   On error, -1 (variable not found, or destination buffer is too small).
//
// Destination buffer is guaranteed to be '\0' - terminated. In case of
// failure, dst[0] == '\0'.
int vz_get_var(const char *data, size_t data_len,
               const char *var_name, char *buf, size_t buf_len);

// Fetch value of certain cookie variable into the destination buffer.
//
// Destination buffer is guaranteed to be '\0' - terminated. In case of
// failure, dst[0] == '\0'. Note that RFC allows many occurrences of the same
// parameter. This function returns only first occurrence.
//
// Return:
//   On success, value length.
//   On error, 0 (either "Cookie:" header is not present at all, or the
//   requested parameter is not found, or destination buffer is too small
//   to hold the value).
int vz_get_cookie(const struct vz_connection *,
                  const char *cookie_name, char *buf, size_t buf_len);


// Return Vizsla version.
const char *vz_version(void);


// MD5 hash given strings.
// Buffer 'buf' must be 33 bytes long. Varargs is a NULL terminated list of
// asciiz strings. When function returns, buf will contain human-readable
// MD5 hash. Example:
//   char buf[33];
//   vz_md5(buf, "aa", "bb", NULL);
void vz_md5(char *buf, ...);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // VIZSLA_HEADER_INCLUDED

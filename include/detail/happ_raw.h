//
// Created by owent on 2016/04/20.
//

#ifndef HIREDIS_HAPP_HIREDIS_HAPP_RAW_H
#define HIREDIS_HAPP_HIREDIS_HAPP_RAW_H

#pragma once

#include <list>
#include <vector>

#include "hiredis_happ_config.h"

#include "happ_connection.h"

namespace hiredis {
namespace happ {
class raw {
 public:
  typedef cmd_exec cmd_t;

  typedef connection connection_t;
  typedef std::unique_ptr<connection_t> connection_ptr_t;

  typedef std::function<void(raw *, connection_t *)> onconnect_fn_t;
  typedef std::function<void(raw *, connection_t *, const struct redisAsyncContext *, int status)> onconnected_fn_t;
  typedef std::function<void(raw *, connection_t *, const struct redisAsyncContext *, int)> ondisconnected_fn_t;
  typedef std::function<void(const char *)> log_fn_t;

  struct config_t {
    connection::key_t init_connection;
    log_fn_t log_fn_info;
    log_fn_t log_fn_debug;
    char *log_buffer;
    size_t log_max_size;

    time_t timer_interval_sec;
    time_t timer_interval_usec;
    time_t timer_timeout_sec;

    size_t cmd_buffer_size;
  };

  struct timer_t {
    time_t last_update_sec;
    time_t last_update_usec;

    struct delay_t {
      time_t sec;
      time_t usec;
      cmd_t *cmd;
    };
    std::list<delay_t> timer_pending;

    struct conn_timetout_t {
      uint64_t sequence;
      time_t timeout;
    };
    conn_timetout_t timer_conn;
  };

 private:
  raw(const raw &);
  raw &operator=(const raw &);

 public:
  HIREDIS_HAPP_API raw();
  HIREDIS_HAPP_API ~raw();

  HIREDIS_HAPP_API int init(const std::string &ip, uint16_t port);

  HIREDIS_HAPP_API const std::string &get_auth_password();
  HIREDIS_HAPP_API void set_auth_password(const std::string &passwd);

  HIREDIS_HAPP_API const connection::auth_fn_t &get_auth_fn();
  HIREDIS_HAPP_API void set_auth_fn(connection::auth_fn_t fn);

  HIREDIS_HAPP_API int start();

  HIREDIS_HAPP_API int reset();

  /**
   * @breif send a request to redis server
   * @param cbk callback
   * @param priv_data private data passed to callback
   * @param argc argument count
   * @param argv pointer of every argument
   * @param argvlen size of every argument
   *
   * @note it can not be used to send subscribe, unsubscribe or monitor command.(because they are
   * not request-response message) hiredis deal with these command without notify event, so you can
   * only use connection::redis_raw_cmd to do these when connection finished or disconnected
   *
   * @see connection::redis_raw_cmd
   * @see connection::redis_cmd
   * @return command wrapper of this message, NULL if failed
   */
  HIREDIS_HAPP_API cmd_t *exec(cmd_t::callback_fn_t cbk, void *priv_data, int argc, const char **argv,
                               const size_t *argvlen);

  /**
   * @breif send a request to redis server
   * @param cbk callback
   * @param priv_data private data passed to callback
   * @param fmt format string
   * @param ... format data
   *
   * @note it can not be used to send subscribe, unsubscribe or monitor command.(because they are
   * not request-response message) hiredis deal with these command without notify event, so you can
   * only use connection::redis_raw_cmd to do these when connection finished or disconnected
   *
   * @see connection::redis_raw_cmd
   * @see connection::redis_cmd
   * @return command wrapper of this message, NULL if failed
   */
  HIREDIS_HAPP_API cmd_t *exec(cmd_t::callback_fn_t cbk, void *priv_data, const char *fmt, ...);

  /**
   * @breif send a request to redis server
   * @param cbk callback
   * @param priv_data private data passed to callback
   * @param fmt format string
   * @param ap format data
   *
   * @note it can not be used to send subscribe, unsubscribe or monitor command.(because they are
   * not request-response message) hiredis deal with these command without notify event, so you can
   * only use connection::redis_raw_cmd to do these when connection finished or disconnected
   *
   * @see connection::redis_raw_cmd
   * @see connection::redis_cmd
   * @return command wrapper of this message, NULL if failed
   */
  HIREDIS_HAPP_API cmd_t *exec(cmd_t::callback_fn_t cbk, void *priv_data, const char *fmt, va_list ap);

  /**
   * @breif send a request to redis server
   * @param cmd cmd wrapper
   *
   * @note it can not be used to send subscribe, unsubscribe or monitor command.(because they are
   * not request-response message) hiredis deal with these command without notify event, so you can
   * only use connection::redis_raw_cmd to do these when connection finished or disconnected
   *
   * @see connection::redis_raw_cmd
   * @see connection::redis_cmd
   * @return command wrapper of this message, NULL if failed
   */
  HIREDIS_HAPP_API cmd_t *exec(cmd_t *cmd);

  /**
   * @breif send a request to specifed redis server
   * @param conn which connect to sent to
   * @param cmd cmd wrapper
   *
   * @note it can not be used to send subscribe, unsubscribe or monitor command.(because they are
   * not request-response message) hiredis deal with these command without notify event, so you can
   * only use connection::redis_raw_cmd to do these when connection finished or disconnected
   *
   * @see connection::redis_raw_cmd
   * @see connection::redis_cmd
   * @return command wrapper of this message, NULL if failed
   */
  HIREDIS_HAPP_API cmd_t *exec(connection_t *conn, cmd_t *cmd);

  /**
   * @breif retry to send a request to redis server
   * @param cmd cmd wrapper
   * @param conn which connect to sent to(pass NULL to try to get one using the key in cmd)
   *
   * @note it can not be used to send subscribe, unsubscribe or monitor command.(because they are
   * not request-response message) hiredis deal with these command without notify event, so you can
   * only use connection::redis_raw_cmd to do these when connection finished or disconnected
   *
   * @see connection::redis_raw_cmd
   * @see connection::redis_cmd
   * @return command wrapper of this message, NULL if failed
   */
  HIREDIS_HAPP_API cmd_t *retry(cmd_t *cmd, connection_t *conn = NULL);

  HIREDIS_HAPP_API const connection_t *get_connection() const;
  HIREDIS_HAPP_API connection_t *get_connection();

  HIREDIS_HAPP_API connection_t *make_connection();
  HIREDIS_HAPP_API bool release_connection(bool close_fd, int status);

  HIREDIS_HAPP_API onconnect_fn_t set_on_connect(onconnect_fn_t cbk);
  HIREDIS_HAPP_API onconnected_fn_t set_on_connected(onconnected_fn_t cbk);
  HIREDIS_HAPP_API ondisconnected_fn_t set_on_disconnected(ondisconnected_fn_t cbk);

  HIREDIS_HAPP_API void set_cmd_buffer_size(size_t s);

  HIREDIS_HAPP_API size_t get_cmd_buffer_size() const;

  HIREDIS_HAPP_API bool is_timer_active() const;

  HIREDIS_HAPP_API void set_timer_interval(time_t sec, time_t usec);

  HIREDIS_HAPP_API void set_timeout(time_t sec);

  HIREDIS_HAPP_API void add_timer_cmd(cmd_t *cmd);

  HIREDIS_HAPP_API int proc(time_t sec, time_t usec);

  HIREDIS_HAPP_API void set_log_writer(log_fn_t info_fn, log_fn_t debug_fn, size_t max_size = 65536);

 private:
  HIREDIS_HAPP_API cmd_t *create_cmd(cmd_t::callback_fn_t cbk, void *pridata);
  HIREDIS_HAPP_API void destroy_cmd(cmd_t *c);
  HIREDIS_HAPP_API int call_cmd(cmd_t *c, int err, redisAsyncContext *context, void *reply);

 private:
  static void on_reply_wrapper(redisAsyncContext *c, void *r, void *privdata);
  static void on_connected_wrapper(const struct redisAsyncContext *, int status);
  static void on_disconnected_wrapper(const struct redisAsyncContext *, int status);

  static void on_reply_auth(cmd_exec *cmd, redisAsyncContext *c, void *r, void *privdata);

  void log_debug(const char *fmt, ...);

  void log_info(const char *fmt, ...);

 private:
  config_t conf_;

  // authorization information
  connection::auth_info_t auth_;

  // current connection
  connection_ptr_t conn_;

  // timers
  timer_t timer_actions_;

  // callbacks_
  struct callback_set_t {
    onconnect_fn_t on_connect;
    onconnected_fn_t on_connected;
    ondisconnected_fn_t on_disconnected;
  };
  callback_set_t callbacks_;
};
}  // namespace happ
}  // namespace hiredis

#endif  // HIREDIS_HAPP_HIREDIS_HAPP_CLUSTER_H
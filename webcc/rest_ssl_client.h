#ifndef WEBCC_REST_SSL_CLIENT_H_
#define WEBCC_REST_SSL_CLIENT_H_

#include "webcc/basic_rest_client.h"
#include "webcc/http_ssl_client.h"

namespace webcc {

typedef BasicRestClient<HttpSslClient> RestSslClient;

}  // namespace webcc

#endif  // WEBCC_REST_SSL_CLIENT_H_

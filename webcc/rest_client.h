#ifndef WEBCC_REST_CLIENT_H_
#define WEBCC_REST_CLIENT_H_

#include "webcc/rest_basic_client.h"
#include "webcc/http_client.h"

namespace webcc {

typedef RestBasicClient<HttpClient> RestClient;

}  // namespace webcc

#endif  // WEBCC_REST_CLIENT_H_

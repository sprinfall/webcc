#ifndef WEBCC_REST_CLIENT_H_
#define WEBCC_REST_CLIENT_H_

#include "webcc/basic_rest_client.h"
#include "webcc/http_client.h"

namespace webcc {

typedef BasicRestClient<HttpClient> RestClient;

}  // namespace webcc

#endif  // WEBCC_REST_CLIENT_H_

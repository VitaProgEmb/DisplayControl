#include "http.h"


//-------------------------------------------------------------
static const char *TAG = "http";
//-------------------------------------------------------------
#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)
//-------------------------------------------------------------
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        return httpd_resp_set_type(req, "application/pdf");
    } else if (IS_FILE_EXT(filename, ".html")) {
        return httpd_resp_set_type(req, "text/html");
    } else if (IS_FILE_EXT(filename, ".jpeg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    } else if (IS_FILE_EXT(filename, ".ico")) {
        return httpd_resp_set_type(req, "image/x-icon");
    }
    return httpd_resp_set_type(req, "text/plain");
}
//-------------------------------------------------------------
static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{

  const size_t base_pathlen = strlen(base_path);
  size_t pathlen = strlen(uri);
  const char *quest = strchr(uri, '?');
  if (quest) {
      pathlen = MIN(pathlen, quest - uri);
  }
  const char *hash = strchr(uri, '#');
  if (hash) {
      pathlen = MIN(pathlen, hash - uri);
  }
  if (base_pathlen + pathlen + 1 > destsize) {
    return NULL;
  }
  strcpy(dest, base_path);
  strlcpy(dest + base_pathlen, uri, pathlen + 1);
  return dest + base_pathlen;
}
//-------------------------------------------------------------
static esp_err_t download_get_handler(httpd_req_t *req)
{
  char filepath[FILE_PATH_MAX];
  FILE *fd = NULL;
  struct stat file_stat;
  const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path, req->uri, sizeof(filepath));
  //=======================================//TODO
  // size_t buf_l = httpd_req_get_url_query_len(req) + 1;
  // if (buf_l > 1)
  // {
  //     char *buf = malloc(buf_l);

  //     if (httpd_req_get_url_query_str(req, buf, buf_l) == ESP_OK)
  //     {
  //       ESP_LOGI(TAG, "Found URL query => %s", buf);
  //       char param[32];
  //       if (httpd_query_key_value(buf, "A", param, sizeof(param)) == ESP_OK) {
  //           ESP_LOGI(TAG, "Found URL query parameter => red:%s", param);
  //           // if(!strcmp(param,"RED+ON"))  ;//TODO this set wifi aut
  //           // else if(!strcmp(param,"RED+OFF");
  //           strcpy(wifi_conf.ap_ssid,param);
            
  //       }
  //       if (httpd_query_key_value(buf, "B", param, sizeof(param)) == ESP_OK) {
  //           ESP_LOGI(TAG, "Found URL query parameter => red:%s", param);
  //           strcpy(wifi_conf.ap_pasw,param);
  //           // if(!strcmp(param,"RED+ON"))  ;
  //           // else if(!strcmp(param,"RED+OFF");
  //           xEventGroupSetBits(EventGroup,RECONECT);
  //       }
          
  //     }
  //     free(buf);
  // }
  //=======================================//TODO
  printf("File path =>%s\n",filename);
  if (!filename) {
      ESP_LOGE(TAG, "Filename is too long");
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
      return ESP_FAIL;
  }
  if (strcmp(filename,"/") == 0) {
    strcat(filepath, "index.html");
  }
  stat(filepath, &file_stat);
  fd = fopen(filepath, "r");
  if (!fd) {
      ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
      /* Respond with 500 Internal Server Error */
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
      return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
  set_content_type_from_file(req, filename);
  char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
  size_t chunksize;
  do {
    chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);
    if (chunksize > 0) {
      if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
        fclose(fd);
        ESP_LOGE(TAG, "File sending failed!");
        httpd_resp_sendstr_chunk(req, NULL);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
        return ESP_FAIL;
      }
    }
  } while (chunksize != 0);
  fclose(fd);
  ESP_LOGI(TAG, "File sending complete");
  httpd_resp_send_chunk(req, NULL, 0);
 

  return ESP_OK;
}
//-------------------------------------------------------------
httpd_handle_t start_webserver(void)
{
  httpd_handle_t server = NULL;

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  static struct file_server_data *server_data = NULL;
  if (server_data) {
      ESP_LOGE(TAG, "File server already started");
      // return ESP_ERR_INVALID_STATE;
      return NULL;
  }
  server_data = calloc(1, sizeof(struct file_server_data));
  if (!server_data) {
      ESP_LOGE(TAG, "Failed to allocate memory for server data");
      // return ESP_ERR_NO_MEM;
      return NULL;
  }
  strlcpy(server_data->base_path, "/spiffs",
           sizeof(server_data->base_path));
  config.uri_match_fn = httpd_uri_match_wildcard;

  config.lru_purge_enable = true;

  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    if (httpd_start(&server, &config) != ESP_OK)
    {
      ESP_LOGI(TAG, "Error starting server!");
      return NULL;
    }

    ESP_LOGI(TAG, "Registering URI handlers");

    httpd_uri_t file_download = {
        .uri       = "/*",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = download_get_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_download);
    return server;
}
//-------------------------------------------------------------
void stop_webserver(httpd_handle_t server)
{
    httpd_stop(server);
}
//-------------------------------------------------------------

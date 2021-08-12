# 1 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
# 15 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2

# 17 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2
# 18 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2
# 19 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2
# 20 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2

# 22 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2
# 23 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2
# 24 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 2

#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7

#define FACE_COLOR_WHITE 0x00FFFFFF
#define FACE_COLOR_BLACK 0x00000000
#define FACE_COLOR_RED 0x000000FF
#define FACE_COLOR_GREEN 0x0000FF00
#define FACE_COLOR_BLUE 0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)

typedef struct {
        size_t size; //number of values used for filtering
        size_t index; //current value index
        size_t count; //value count
        int sum;
        int * values; //array to be filled with values
} ra_filter_t;

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" "123456789000000000000987654321";
static const char* _STREAM_BOUNDARY = "\r\n--" "123456789000000000000987654321" "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static ra_filter_t ra_filter;
httpd_handle_t stream_httpd = 
# 56 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                             __null
# 56 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                 ;
httpd_handle_t camera_httpd = 
# 57 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                             __null
# 57 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                 ;

static mtmn_config_t mtmn_config = {0};
static int8_t detection_enabled = 0;
static int8_t recognition_enabled = 0;
static int8_t is_enrolling = 0;
static face_id_list id_list = {0};

static ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size){
    memset(filter, 0, sizeof(ra_filter_t));

    filter->values = (int *)malloc(sample_size * sizeof(int));
    if(!filter->values){
        return 
# 70 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
              __null
# 70 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                  ;
    }
    memset(filter->values, 0, sample_size * sizeof(int));

    filter->size = sample_size;
    return filter;
}

static int ra_filter_run(ra_filter_t * filter, int value){
    if(!filter->values){
        return value;
    }
    filter->sum -= filter->values[filter->index];
    filter->values[filter->index] = value;
    filter->sum += filter->values[filter->index];
    filter->index++;
    filter->index = filter->index % filter->size;
    if (filter->count < filter->size) {
        filter->count++;
    }
    return filter->sum / filter->count;
}

static void rgb_print(dl_matrix3du_t *image_matrix, uint32_t color, const char * str){
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, 10, color, str);
}

static int rgb_printf(dl_matrix3du_t *image_matrix, uint32_t color, const char *format, ...){
    char loc_buf[64];
    char * temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    
# 109 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   __builtin_va_start(
# 109 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   arg
# 109 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   ,
# 109 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   format
# 109 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   )
# 109 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                        ;
    
# 110 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   __builtin_va_copy(
# 110 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   copy
# 110 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   ,
# 110 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   arg
# 110 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   )
# 110 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                     ;
    len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    
# 112 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   __builtin_va_end(
# 112 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   copy
# 112 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   )
# 112 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
               ;
    if(len >= sizeof(loc_buf)){
        temp = (char*)malloc(len+1);
        if(temp == 
# 115 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                  __null
# 115 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                      ) {
            return 0;
        }
    }
    vsnprintf(temp, len+1, format, arg);
    
# 120 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   __builtin_va_end(
# 120 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   arg
# 120 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
   )
# 120 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
              ;
    rgb_print(image_matrix, color, temp);
    if(len > 64){
        free(temp);
    }
    return len;
}

static void draw_face_boxes(dl_matrix3du_t *image_matrix, box_array_t *boxes, int face_id){
    int x, y, w, h, i;
    uint32_t color = (0x000000FF | 0x0000FF00);
    if(face_id < 0){
        color = 0x000000FF;
    } else if(face_id > 0){
        color = 0x0000FF00;
    }
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    for (i = 0; i < boxes->len; i++){
        // rectangle box
        x = (int)boxes->box[i].box_p[0];
        y = (int)boxes->box[i].box_p[1];
        w = (int)boxes->box[i].box_p[2] - x + 1;
        h = (int)boxes->box[i].box_p[3] - y + 1;
        fb_gfx_drawFastHLine(&fb, x, y, w, color);
        fb_gfx_drawFastHLine(&fb, x, y+h-1, w, color);
        fb_gfx_drawFastVLine(&fb, x, y, h, color);
        fb_gfx_drawFastVLine(&fb, x+w-1, y, h, color);
# 161 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
    }
}

static int run_face_recognition(dl_matrix3du_t *image_matrix, box_array_t *net_boxes){
    dl_matrix3du_t *aligned_face = 
# 165 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                                  __null
# 165 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                      ;
    int matched_id = 0;

    aligned_face = dl_matrix3du_alloc(1, 56, 56, 3);
    if(!aligned_face){
        Serial.println("Could not allocate face recognition buffer");
        return matched_id;
    }
    if (align_face(net_boxes, image_matrix, aligned_face) == 0 /*!< esp_err_t value indicating success (no error) */){
        if (is_enrolling == 1){
            int8_t left_sample_face = enroll_face(&id_list, aligned_face);

            if(left_sample_face == (5 - 1)){
                Serial.printf("Enrolling Face ID: %d\n", id_list.tail);
            }
            Serial.printf("Enrolling Face ID: %d sample %d\n", id_list.tail, 5 - left_sample_face);
            rgb_printf(image_matrix, (0x00FF0000 | 0x0000FF00), "ID[%u] Sample[%u]", id_list.tail, 5 - left_sample_face);
            if (left_sample_face == 0){
                is_enrolling = 0;
                Serial.printf("Enrolled Face ID: %d\n", id_list.tail);
            }
        } else {
            matched_id = recognize_face(&id_list, aligned_face);
            if (matched_id >= 0) {
                Serial.printf("Match Face ID: %u\n", matched_id);
                rgb_printf(image_matrix, 0x0000FF00, "Hello Subject %u", matched_id);
            } else {
                Serial.println("No Match Found");
                rgb_print(image_matrix, 0x000000FF, "Intruder Alert!");
                matched_id = -1;
            }
        }
    } else {
        Serial.println("Face Not Aligned");
        //rgb_print(image_matrix, FACE_COLOR_YELLOW, "Human Detected");
    }

    dl_matrix3du_free(aligned_face);
    return matched_id;
}

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != 0 /*!< esp_err_t value indicating success (no error) */){
        return 0;
    }
    j->len += len;
    return len;
}

static esp_err_t capture_handler(httpd_req_t *req){
    camera_fb_t * fb = 
# 219 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                      __null
# 219 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                          ;
    esp_err_t res = 0 /*!< esp_err_t value indicating success (no error) */;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        httpd_resp_send_500(req);
        return -1 /*!< Generic esp_err_t code indicating failure */;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t out_len, out_width, out_height;
    uint8_t * out_buf;
    bool s;
    bool detected = false;
    int face_id = 0;
    if(!detection_enabled || fb->width > 400){
        size_t fb_len = 0;
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?0 /*!< esp_err_t value indicating success (no error) */:-1 /*!< Generic esp_err_t code indicating failure */;
            httpd_resp_send_chunk(req, 
# 247 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                                      __null
# 247 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                          , 0);
            fb_len = jchunk.len;
        }
        esp_camera_fb_return(fb);
        int64_t fr_end = esp_timer_get_time();
        Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start)/1000));
        return res;
    }

    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    if (!image_matrix) {
        esp_camera_fb_return(fb);
        Serial.println("dl_matrix3du_alloc failed");
        httpd_resp_send_500(req);
        return -1 /*!< Generic esp_err_t code indicating failure */;
    }

    out_buf = image_matrix->item;
    out_len = fb->width * fb->height * 3;
    out_width = fb->width;
    out_height = fb->height;

    s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
    esp_camera_fb_return(fb);
    if(!s){
        dl_matrix3du_free(image_matrix);
        Serial.println("to rgb888 failed");
        httpd_resp_send_500(req);
        return -1 /*!< Generic esp_err_t code indicating failure */;
    }

    box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

    if (net_boxes){
        detected = true;
        if(recognition_enabled){
            face_id = run_face_recognition(image_matrix, net_boxes);
        }
        draw_face_boxes(image_matrix, net_boxes, face_id);
        free(net_boxes->score);
        free(net_boxes->box);
        free(net_boxes->landmark);
        free(net_boxes);
    }

    jpg_chunking_t jchunk = {req, 0};
    s = fmt2jpg_cb(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);
    dl_matrix3du_free(image_matrix);
    if(!s){
        Serial.println("JPEG compression failed");
        return -1 /*!< Generic esp_err_t code indicating failure */;
    }

    int64_t fr_end = esp_timer_get_time();
    Serial.printf("FACE: %uB %ums %s%d\n", (uint32_t)(jchunk.len), (uint32_t)((fr_end - fr_start)/1000), detected?"DETECTED ":"", face_id);
    return res;
}

static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = 
# 306 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                      __null
# 306 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                          ;
    esp_err_t res = 0 /*!< esp_err_t value indicating success (no error) */;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = 
# 309 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                        __null
# 309 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                            ;
    char * part_buf[64];
    dl_matrix3du_t *image_matrix = 
# 311 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                                  __null
# 311 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                      ;
    bool detected = false;
    int face_id = 0;
    int64_t fr_start = 0;
    int64_t fr_ready = 0;
    int64_t fr_face = 0;
    int64_t fr_recognize = 0;
    int64_t fr_encode = 0;

    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != 0 /*!< esp_err_t value indicating success (no error) */){
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while(true){
        detected = false;
        face_id = 0;
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = -1 /*!< Generic esp_err_t code indicating failure */;
        } else {
            fr_start = esp_timer_get_time();
            fr_ready = fr_start;
            fr_face = fr_start;
            fr_encode = fr_start;
            fr_recognize = fr_start;
            if(!detection_enabled || fb->width > 400){
                if(fb->format != PIXFORMAT_JPEG){
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = 
# 349 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                        __null
# 349 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                            ;
                    if(!jpeg_converted){
                        Serial.println("JPEG compression failed");
                        res = -1 /*!< Generic esp_err_t code indicating failure */;
                    }
                } else {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
            } else {

                image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

                if (!image_matrix) {
                    Serial.println("dl_matrix3du_alloc failed");
                    res = -1 /*!< Generic esp_err_t code indicating failure */;
                } else {
                    if(!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)){
                        Serial.println("fmt2rgb888 failed");
                        res = -1 /*!< Generic esp_err_t code indicating failure */;
                    } else {
                        fr_ready = esp_timer_get_time();
                        box_array_t *net_boxes = 
# 371 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                                                __null
# 371 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                                    ;
                        if(detection_enabled){
                            net_boxes = face_detect(image_matrix, &mtmn_config);
                        }
                        fr_face = esp_timer_get_time();
                        fr_recognize = fr_face;
                        if (net_boxes || fb->format != PIXFORMAT_JPEG){
                            if(net_boxes){
                                detected = true;
                                if(recognition_enabled){
                                    face_id = run_face_recognition(image_matrix, net_boxes);
                                }
                                fr_recognize = esp_timer_get_time();
                                draw_face_boxes(image_matrix, net_boxes, face_id);
                                free(net_boxes->score);
                                free(net_boxes->box);
                                free(net_boxes->landmark);
                                free(net_boxes);
                            }
                            if(!fmt2jpg(image_matrix->item, fb->width*fb->height*3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len)){
                                Serial.println("fmt2jpg failed");
                                res = -1 /*!< Generic esp_err_t code indicating failure */;
                            }
                            esp_camera_fb_return(fb);
                            fb = 
# 395 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                                __null
# 395 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                    ;
                        } else {
                            _jpg_buf = fb->buf;
                            _jpg_buf_len = fb->len;
                        }
                        fr_encode = esp_timer_get_time();
                    }
                    dl_matrix3du_free(image_matrix);
                }
            }
        }
        if(res == 0 /*!< esp_err_t value indicating success (no error) */){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == 0 /*!< esp_err_t value indicating success (no error) */){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == 0 /*!< esp_err_t value indicating success (no error) */){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(fb){
            esp_camera_fb_return(fb);
            fb = 
# 418 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                __null
# 418 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                    ;
            _jpg_buf = 
# 419 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                      __null
# 419 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                          ;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = 
# 422 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                      __null
# 422 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                          ;
        }
        if(res != 0 /*!< esp_err_t value indicating success (no error) */){
            break;
        }
        int64_t fr_end = esp_timer_get_time();

        int64_t ready_time = (fr_ready - fr_start)/1000;
        int64_t face_time = (fr_face - fr_ready)/1000;
        int64_t recognize_time = (fr_recognize - fr_face)/1000;
        int64_t encode_time = (fr_encode - fr_recognize)/1000;
        int64_t process_time = (fr_encode - fr_start)/1000;

        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        Serial.printf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps), %u+%u+%u+%u=%u %s%d\n",
            (uint32_t)(_jpg_buf_len),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
            avg_frame_time, 1000.0 / avg_frame_time,
            (uint32_t)ready_time, (uint32_t)face_time, (uint32_t)recognize_time, (uint32_t)encode_time, (uint32_t)process_time,
            (detected)?"DETECTED ":"", face_id
        );
    }

    last_frame = 0;
    return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
    char* buf;
    size_t buf_len;
    char variable[32] = {0,};
    char value[32] = {0,};

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return -1 /*!< Generic esp_err_t code indicating failure */;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == 0 /*!< esp_err_t value indicating success (no error) */) {
            if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == 0 /*!< esp_err_t value indicating success (no error) */ &&
                httpd_query_key_value(buf, "val", value, sizeof(value)) == 0 /*!< esp_err_t value indicating success (no error) */) {
            } else {
                free(buf);
                httpd_resp_send_404(req);
                return -1 /*!< Generic esp_err_t code indicating failure */;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return -1 /*!< Generic esp_err_t code indicating failure */;
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return -1 /*!< Generic esp_err_t code indicating failure */;
    }

    int val = atoi(value);
    sensor_t * s = esp_camera_sensor_get();
    int res = 0;

    if(!strcmp(variable, "framesize")) {
        if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
    }
    else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);
    else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);
    else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);
    else if(!strcmp(variable, "saturation")) res = s->set_saturation(s, val);
    else if(!strcmp(variable, "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)val);
    else if(!strcmp(variable, "colorbar")) res = s->set_colorbar(s, val);
    else if(!strcmp(variable, "awb")) res = s->set_whitebal(s, val);
    else if(!strcmp(variable, "agc")) res = s->set_gain_ctrl(s, val);
    else if(!strcmp(variable, "aec")) res = s->set_exposure_ctrl(s, val);
    else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, val);
    else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, val);
    else if(!strcmp(variable, "awb_gain")) res = s->set_awb_gain(s, val);
    else if(!strcmp(variable, "agc_gain")) res = s->set_agc_gain(s, val);
    else if(!strcmp(variable, "aec_value")) res = s->set_aec_value(s, val);
    else if(!strcmp(variable, "aec2")) res = s->set_aec2(s, val);
    else if(!strcmp(variable, "dcw")) res = s->set_dcw(s, val);
    else if(!strcmp(variable, "bpc")) res = s->set_bpc(s, val);
    else if(!strcmp(variable, "wpc")) res = s->set_wpc(s, val);
    else if(!strcmp(variable, "raw_gma")) res = s->set_raw_gma(s, val);
    else if(!strcmp(variable, "lenc")) res = s->set_lenc(s, val);
    else if(!strcmp(variable, "special_effect")) res = s->set_special_effect(s, val);
    else if(!strcmp(variable, "wb_mode")) res = s->set_wb_mode(s, val);
    else if(!strcmp(variable, "ae_level")) res = s->set_ae_level(s, val);
    else if(!strcmp(variable, "face_detect")) {
        detection_enabled = val;
        if(!detection_enabled) {
            recognition_enabled = 0;
        }
    }
    else if(!strcmp(variable, "face_enroll")) is_enrolling = val;
    else if(!strcmp(variable, "face_recognize")) {
        recognition_enabled = val;
        if(recognition_enabled){
            detection_enabled = val;
        }
    }
    else {
        res = -1;
    }

    if(res){
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, 
# 536 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                               __null
# 536 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                                   , 0);
}

static esp_err_t status_handler(httpd_req_t *req){
    static char json_response[1024];

    sensor_t * s = esp_camera_sensor_get();
    char * p = json_response;
    *p++ = '{';

    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p+=sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
    p+=sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
    p+=sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
    p+=sprintf(p, "\"awb\":%u,", s->status.awb);
    p+=sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
    p+=sprintf(p, "\"aec\":%u,", s->status.aec);
    p+=sprintf(p, "\"aec2\":%u,", s->status.aec2);
    p+=sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p+=sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p+=sprintf(p, "\"agc\":%u,", s->status.agc);
    p+=sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p+=sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
    p+=sprintf(p, "\"bpc\":%u,", s->status.bpc);
    p+=sprintf(p, "\"wpc\":%u,", s->status.wpc);
    p+=sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
    p+=sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p+=sprintf(p, "\"vflip\":%u,", s->status.vflip);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p+=sprintf(p, "\"dcw\":%u,", s->status.dcw);
    p+=sprintf(p, "\"colorbar\":%u,", s->status.colorbar);
    p+=sprintf(p, "\"face_detect\":%u,", detection_enabled);
    p+=sprintf(p, "\"face_enroll\":%u,", is_enrolling);
    p+=sprintf(p, "\"face_recognize\":%u", recognition_enabled);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == (0x36)) {
        return httpd_resp_send(req, (const char *)index_ov3660_html_gz, 4408);
    }
    return httpd_resp_send(req, (const char *)index_ov2640_html_gz, 4316);
}

void startCameraServer(){
    httpd_config_t config = { .task_priority = ( ( UBaseType_t ) 0U )+5, .stack_size = 4096, .server_port = 80, .ctrl_port = 32768, .max_open_sockets = 7, .max_uri_handlers = 8, .max_resp_headers = 8, .backlog_conn = 5, .lru_purge_enable = false, .recv_wait_timeout = 5, .send_wait_timeout = 5, .global_user_ctx = 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                           __null
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                           , .global_user_ctx_free_fn = 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                           __null
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                           , .global_transport_ctx = 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                           __null
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                           , .global_transport_ctx_free_fn = 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                           __null
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                           , .open_fn = 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                           __null
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                           , .close_fn = 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                           __null
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                           , .uri_match_fn = 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                           __null 
# 592 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
                           };

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = 
# 598 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                    __null
    
# 599 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   };

    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = 
# 605 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                    __null
    
# 606 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   };

    httpd_uri_t cmd_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler,
        .user_ctx = 
# 612 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                    __null
    
# 613 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   };

    httpd_uri_t capture_uri = {
        .uri = "/capture",
        .method = HTTP_GET,
        .handler = capture_handler,
        .user_ctx = 
# 619 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                    __null
    
# 620 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   };

   httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = 
# 626 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp" 3 4
                    __null
    
# 627 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\app_httpd.cpp"
   };


    ra_filter_init(&ra_filter, 20);

    mtmn_config.type = FAST;
    mtmn_config.min_face = 80;
    mtmn_config.pyramid = 0.707;
    mtmn_config.pyramid_times = 4;
    mtmn_config.p_threshold.score = 0.6;
    mtmn_config.p_threshold.nms = 0.7;
    mtmn_config.p_threshold.candidate_number = 20;
    mtmn_config.r_threshold.score = 0.7;
    mtmn_config.r_threshold.nms = 0.7;
    mtmn_config.r_threshold.candidate_number = 10;
    mtmn_config.o_threshold.score = 0.7;
    mtmn_config.o_threshold.nms = 0.7;
    mtmn_config.o_threshold.candidate_number = 1;

    face_id_init(&id_list, 7, 5);

    Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&camera_httpd, &config) == 0 /*!< esp_err_t value indicating success (no error) */) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == 0 /*!< esp_err_t value indicating success (no error) */) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}
# 1 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\device.ino"

# 3 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\device.ino" 2

//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER 

# 17 "c:\\Users\\Pups\\source\\repos\\projects\\IoTSecurityCamera\\Device\\device.ino" 2

const char* ssid = "HappyWIFI-EXT";
const char* password = "H1234567i!";

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }






  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != 0 /*!< esp_err_t value indicating success (no error) */) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == (0x36)) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);






  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}

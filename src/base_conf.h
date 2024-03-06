#ifndef INCLUDE_BASE_CONF_H
#define INCLUDE_BASE_CONF_H

/**
 * Contiene le costanti di configurazione
 */

#define CONF_ANALOG_READ_MAX              (4095)

#define CONF_DHT
#ifdef CONF_DHT
#define CONF_DHT_READ_INTERVAL_MIN        (5000)
#endif

#define CONF_LOG_LEVEL                    "i"

#define CONF_MONITOR_BAUD_RATE            (115200)

#define CONF_SCHEMA_PIN_COUNT             (64)

// Valore per Europe/Rome in https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 
#define CONF_TIME_ZONE                    "CET-1CEST,M3.5.0,M10.5.0/3"

#define CONF_WIFI
#ifdef CONF_WIFI

#define CONF_WEB
#ifdef CONF_WEB
#define CONF_WEB_ADMIN_PASSWORD           "admin"
#define CONF_WEB_ADMIN_USERNAME           "admin"
#define CONF_WEB_HTML_TITLE               "ESP32"
#define CONF_WEB_HTTP_PORT                (80)
#define CONF_WEB_HTTPS
#ifdef CONF_WEB_HTTPS
#define CONF_WEB_HTTPS_CERT               "MIICGDCCAYECAQIwDQYJKoZIhvcNAQELBQAwVDELMAkGA1UEBhMCREUxCzAJBgNVBAgMAkJFMQ8wDQYDVQQHDAZCZXJsaW4xEjAQBgNVBAoMCU15Q29tcGFueTETMBEGA1UEAwwKbXljYS5sb2NhbDAeFw0yNDAyMTIxMjAwNTlaFw0zNDAyMDkxMjAwNTlaMFUxCzAJBgNVBAYTAkRFMQswCQYDVQQIDAJCRTEPMA0GA1UEBwwGQmVybGluMRIwEAYDVQQKDAlNeUNvbXBhbnkxFDASBgNVBAMMC2VzcDMyLmxvY2FsMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDND5F9H6zW+hIBsy3sV7/eE26Cp6pXBA37M8GGeFXYoeLHzNffdXYdXFQONIjSOxn5X3gTBY2YppBvgzH9TCiN8Y2tXH2r6QpBnZrfxJBQAP1RmNReE6RTJ+l/4tJ8LLBe7Hs+jyCq9sFwF8b09thrUb2q1IwrlK+tOa8sW77HXwIDAQABMA0GCSqGSIb3DQEBCwUAA4GBAGh9FURupOK9PrQvLvH68/3MA3ZGkuO7V+xHY9G5lEMrg5fFY3zKJt31gYL9PXyEcpQwJd9+NXDneBrKWXs5Bxoy/sEIKT8BDPqHc0SRoAFnznvfh9c218YfaqcypeVtv37v35V6tQeOwCsr54HTBjZlFCPT4IxEwP0+Ya5bpHwB"
#define CONF_WEB_HTTPS_CERT_KEY           "MIICXgIBAAKBgQDND5F9H6zW+hIBsy3sV7/eE26Cp6pXBA37M8GGeFXYoeLHzNffdXYdXFQONIjSOxn5X3gTBY2YppBvgzH9TCiN8Y2tXH2r6QpBnZrfxJBQAP1RmNReE6RTJ+l/4tJ8LLBe7Hs+jyCq9sFwF8b09thrUb2q1IwrlK+tOa8sW77HXwIDAQABAoGBAK2MoYuORU/kcVzy8sj2MzKWq470rBvdGKAj24QdUDp6OF71JTbASOjsfU9QcwPKdV8yx4MBzicFwXQ/Sw2faSnEQtnJIanYhXsV8PZh62h5mfbnQPEHTzoY/LN/JXxn5e4hMNm6ruUBr0OdcOvxvAvRiBEC4m6xzBYlc2KRKW4xAkEA9H9envByR8JpUqNWwv9ah1Ou2rKkJGzdTYivfEIzEUs20dplO7pmWgq5mq9wmFzYWr+xtvSjHtFMsDulwrTsTQJBANa1PRTzwdloUildxmDPuiAQ0PAI22UtSAxlib0sJE5sAX7MHB0Eej2r3E3msESi4/VgkM1gzhs6H4HryRzt6FsCQQClY3NyOFxVxmDQkcUi6vEmEal6LtVx/mJFDG0ItQ8uZ56RnhZUrOaHijG7PWoA5u1DleB1Tk75jcM+g1rBtvkRAkEAh4571xujGwE7RnS3R+4+w/n6AGKHKhTxtSnyUaZp8b1NIGH7qfJmv0MfX2uzhFIXfJeQNKrW1efXdq6IFiS6mwJAX+w1DIXu44zFSgeiexFJQ7yNInI6vOLGUHb1i1v+/m5Xscy7TASIMLACJFculTRvvmlbQot0kSpb+RGs03OMfw=="
#define CONF_WEB_HTTPS_PORT               (443)
#define CONF_WEB_HTTPS_NAME               "esp32"
#endif
#define CONF_WEB_UPLOAD_LIMIT             (10240)
#define CONF_WEB_URI_CONFIG               "/config"
#define CONF_WEB_URI_CONFIG_RESET         "/config/reset"
#define CONF_WEB_URI_CONFIG_UPLOAD        "/config/upload"
#define CONF_WEB_URI_FIRMWARE_UPDATE      "/firmware/update"
#define CONF_WEB_URI_RESET                "/reset"
#endif

#define CONF_WIFI_AP_IP                   "192.168.32.1"
#define CONF_WIFI_AP_PSWD                 "esp32"
#define CONF_WIFI_AP_SSID                 "esp32"
#define CONF_WIFI_CHECK_INTERVAL          (60000)
#define CONF_WIFI_CHECK_INTERVAL_MIN      (60000)
#define CONF_WIFI_CHECK_THRESHOLD         (300000)
#define CONF_WIFI_CONN_TIMEOUT_MS         (10000)
#define CONF_WIFI_COUNT                   (3)
#define CONF_WIFI_MODE                    (1)
#define CONF_WIFI_MODE_LIMIT              (180000)
#define CONF_WIFI_NAME                    "esp32"
#define CONF_WIFI_NTP_SERVER              "pool.ntp.org"
#define CONF_WIFI_NTP_INTERVAL            (300000)

#endif

#endif

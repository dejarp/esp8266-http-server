#include "ets_sys.h"
#include "osapi.h"
#include "uart.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "http_parser.h"

static const int pin = 2;
static volatile os_timer_t some_timer;
// connection must be defined globally because if you define it within the scope of
// a function it will leave scope, but things will still be attempting to access it
struct espconn connection;

void ICACHE_FLASH_ATTR system_init_callback() {
  os_printf("System Initialized\n");
  os_printf("SDK Version: %s\n", system_get_sdk_version());
  os_printf("Chip ID: %u\n", system_get_chip_id());
  os_printf("Voltage: %u\n", system_get_vdd33());
  os_printf("Free Heap Size: %u\n", system_get_free_heap_size());
  os_printf("Boot Version: %u\n", system_get_boot_version());
  os_printf("Base addr of user bin: %u\n", system_get_userbin_addr());
  os_printf("CPU Freq: %u\n", system_get_cpu_freq());
  os_printf("Flash Size: %d\n", system_get_flash_size_map());
  struct rst_info *ri = system_get_rst_info();
  os_printf("reason: %x, exccause: %x, epc1: %x, epc2: %x, epc3: %x, excvaddr: %x, depc: %x\n", ri->reason, ri->exccause, ri->epc1, ri->epc2, ri->epc3, ri->excvaddr, ri->depc);

}

void ICACHE_FLASH_ATTR some_timerfunc(void *arg)
{ 
  //Do blinky stuff
  if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin))
  {
    // set gpio low
    gpio_output_set(0, (1 << pin), 0, 0);
  }
  else
  {
    // set gpio high
    gpio_output_set((1 << pin), 0, 0, 0);
  }
}

void ICACHE_FLASH_ATTR tcp_disconnect(void *arg) {
  os_printf( "tcp connection terminated\n" );
}


char* response = "HTTP/1.1 200 OK\nContent-Length: 100\nContent-Type: text/html\nConnection: Closed\n\n<html><body><h1>Hello, World!</h1></body></html>\n";

http_parser_settings settings;
http_parser parser;
void ICACHE_FLASH_ATTR tcp_receive(void *arg, char *data, unsigned short length) {
  struct espconn *connection = arg;
  os_printf( "data received\n" );
  os_printf( "%s\n", data );

  http_parser_execute(&parser, &settings, data, length);

  os_printf( "Attempting to send response:\n%s\n", response );

  uint16 errorCode = espconn_send( connection, (uint8*)response, os_strlen(response) ); 
  if( errorCode  ) {
    os_printf( "failed to send response, reason: %d\n", errorCode );
  }
  os_printf( "sent response\n");
}

int ICACHE_FLASH_ATTR on_message_begin(http_parser* parser) {
	os_printf( "on_message_begin:\n");
	return 0;
}

int ICACHE_FLASH_ATTR on_url_parsed(http_parser* parser, const char *url, size_t length) {
	os_printf( "on_url: %s\n", url);
	return 0;
}

int ICACHE_FLASH_ATTR on_status(http_parser* parser, const char *status, size_t length) {
	os_printf( "on_status: %s\n", status);
	return 0;
}

int ICACHE_FLASH_ATTR on_header_field(http_parser* parser, const char *header, size_t length) {
	char headerField[length+1];
	os_strncpy(headerField, header, length);
	headerField[length] = '\0';
	os_printf( "on_header_field: %s\n", length, headerField );
	return 0;
}

int ICACHE_FLASH_ATTR on_header_value(http_parser* parser, const char *value, size_t length) {
	os_printf( "on_header_value: %s\n", value );
	return 0;
}

int ICACHE_FLASH_ATTR on_headers_complete(http_parser* parser) {
	os_printf( "on_headers_complete:\n" );
	return 0;
}

int ICACHE_FLASH_ATTR on_body(http_parser* parser, const char *body, size_t length) {
	os_printf( "on_body: %s\n", body );
	return 0;
}

int ICACHE_FLASH_ATTR on_message_complete(http_parser* parser) {
	os_printf( "on_message_complete:\n" );
	return 0;
}

int ICACHE_FLASH_ATTR on_chunk_header(http_parser* parser) {
	os_printf( "on_chunk_header:\n" );
	return 0;
}

int ICACHE_FLASH_ATTR on_chunk_complete(http_parser* parser) {
	os_printf( "on_chunk_complete:\n" );
	return 0;
}

void ICACHE_FLASH_ATTR tcp_connected(void *arg) {
  struct espconn *connection = (struct espconn *)arg;

  os_printf("TCP connection established\n");

  settings.on_message_begin = on_message_begin;
  settings.on_url = on_url_parsed;
  settings.on_status = on_status;
  settings.on_header_field = on_header_field;
  settings.on_header_value = on_header_value;
  settings.on_headers_complete = on_headers_complete;
  settings.on_body = on_body;
  settings.on_message_complete = on_message_complete;
  settings.on_chunk_header = on_chunk_header;
  settings.on_chunk_complete = on_chunk_complete;

  http_parser_init(&parser, HTTP_REQUEST);

  espconn_regist_recvcb(connection, tcp_receive);
  espconn_regist_disconcb(connection, tcp_disconnect);
} 

void ICACHE_FLASH_ATTR tcp_init() {
  // Steps to initialize according to SDK docs
  // 1. Initialize espconn paramters according to protocols
  // 2. Register connect callback function and reconnect callback function
  //   - espconn_regist_connectcb
  //   - espconn_regist_reconcb
  // 3. Call espconn_accept to listen to the connection with host..
  // 4. Registered connect function will be called after a successful connection,
  //    which will register a corresponding callback function.
  //   - Call espconn_regist_recvcb, espconn_regist_sentcb and espconn_regist_disconcb
  //     in the connect callback
  connection.type = ESPCONN_TCP;
  connection.state = ESPCONN_NONE;
  connection.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
  connection.proto.tcp->local_port = 80;
  espconn_regist_connectcb(&connection, tcp_connected);
  espconn_accept(&connection);
  os_printf( "TCP Server Started...\n" );
}

void ICACHE_FLASH_ATTR user_init()
{
  uart_init(BIT_RATE_115200,BIT_RATE_115200);

  system_init_done_cb(system_init_callback);

  tcp_init();

  gpio_init();
  gpio_output_set(0, 0, (1 << pin), 0);

  os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);
  os_timer_arm(&some_timer, 500, 1);
}

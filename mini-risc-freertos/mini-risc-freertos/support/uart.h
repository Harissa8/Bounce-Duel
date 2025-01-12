#ifndef UART_H
#define UART_H

#include <sys/types.h>

void    init_uart();
ssize_t uart_read(char *ptr, size_t len);
ssize_t uart_write(const char *ptr, size_t len);

#endif /* UART_H */

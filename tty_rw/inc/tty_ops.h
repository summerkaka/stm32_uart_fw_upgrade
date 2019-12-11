

extern int tty_init(char *);
int tty_send_command(int tty_fd, uint8_t *pdata, uint8_t count, uint32_t time_out);
int tty_get_response(int tty_fd, uint8_t *pdata, uint8_t count, uint32_t time_out);
CC = avr-gcc
CFLAGS = -Os -mmcu=atmega8

snake.hex: snake.elf
	avr-objcopy -O ihex -R .eeprom snake.elf snake.hex

snake.o: snake.c
	$(CC) -c $(CFLAGS) snake.c

snake.elf: snake.o ../5110_lcd_driver/lcd_driver.o
	$(CC) $(CFLAGS) snake.o ../5110_lcd_driver/lcd_driver.o -o snake.elf



programm: snake.hex
	sudo avrdude -c avrispv2 -P usb -p m8 -U flash:w:snake.hex
clean:
	rm snake.o
	rm snake.elf
	rm snake.hex

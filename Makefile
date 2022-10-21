
TARGET = reader modifier


.PHONY: clean
clean:
	rm -rf *.o
	rm -rf $(TARGET)

$(TARGET) : % : %.o
	$(CC) -o $@ $< -lelf

%.o : %.c
	$(CC) -c -g $<

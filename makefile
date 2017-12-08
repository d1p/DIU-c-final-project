build: main.c
	gcc main.c -std=gnu99 -o puzzle
clean:
	rm -rf *.o a.out core log.txt bin obj puzzle

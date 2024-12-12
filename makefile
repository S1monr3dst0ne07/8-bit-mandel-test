

run: 
	gcc -lm mandel.c
	./a.out

test:
	gcc -lm test.c
	./a.out

lint:
	ls mandel.c | entr -s "make run"

view:
	feh -R 1 new.ppm -Z --force-aliasing &

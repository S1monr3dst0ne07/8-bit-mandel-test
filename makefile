

run: 
	gcc -lm mandel.c -o a
	./a

test:
	gcc -lm test.c -o a
	./a

lint:
	ls mandel.c | entr -s "make run"

view:
	feh -R 1 new.ppm -Z --force-aliasing &

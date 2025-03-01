all: compile test

compile:
	javac Triangle.java
	javac -cp .:/homes/cs408/Spring2025/junit_launch.jar TriangleTest.java

test: Triangle.class TriangleTest.class
	java -jar /homes/cs408/Spring2025/junit_launch.jar -cp . --scan-class-path > output.txt

clean:
	rm Triangle.class TriangleTest.class

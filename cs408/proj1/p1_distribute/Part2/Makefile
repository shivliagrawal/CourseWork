JUNIT_JAR=/homes/cs408/Spring2025/junit_launch.jar
SRC=Triangle.java TriangleTest.java
TEST_OUTPUT=output.txt

all: compile run

compile:
	javac -cp .:$(JUNIT_JAR) $(SRC)

run: compile
	java -jar $(JUNIT_JAR) -cp . --scan-class-path > $(TEST_OUTPUT)

clean:
	rm -f *.class $(TEST_OUTPUT)


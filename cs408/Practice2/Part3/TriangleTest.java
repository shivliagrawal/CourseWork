import static org.junit.jupiter.api.Assertions.*;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.*;
import org.junit.jupiter.api.Test;

class TriangleTest {     //Complete the rest

	@ParameterizedTest
	@CsvFileSource(resources ="cases.csv") 
	void test (int a, int b, int c, String text, String exp) {
		if (exp.equals("exp")) {
			
			Exception e = assertThrows(IllegalArgumentException.class, () -> Triangle.triangle(a,b,c));
			assertEquals(text, e.getMessage());
		}

		else {
			assertEquals(Integer.parseInt(text), Triangle.triangle(a,b,c));
		}
	}

}

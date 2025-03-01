import static org.junit.jupiter.api.Assertions.*;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.*;
import org.junit.jupiter.api.Test;

class TriangleTest {     //Complete the rest

	@ParameterizedTest
	@CsvFileSource(resources ="cases.csv")
	void test (int a, int b, int c, int exp) {
		assertEquals(exp, Triangle.triangle(a,b,c));
	}

	@Test
	void testRange() {
		Exception e1 = assertThrows(IllegalArgumentException.class, () -> Triangle.triangle(100,100,201));
		assertEquals("Out of Range", e1.getMessage());

		Exception e2 = assertThrows(IllegalArgumentException.class, () -> Triangle.triangle(201,201,201));
                assertEquals("Out of Range", e2.getMessage());
	}


}

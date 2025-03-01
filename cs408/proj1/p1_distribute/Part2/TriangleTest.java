import static org.junit.jupiter.api.Assertions.assertEquals;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

public class TriangleTest {

    @ParameterizedTest
    @CsvSource({
        "150, 200, 130, 0",   
        "120, 120, 100, 1",   
        "2, 50, 2, -1"        
    })
    public void testTriangle(int a, int b, int c, int expected) {
        Triangle triangle = new Triangle();
        assertEquals(expected, triangle.triangle(a, b, c));
    }
}


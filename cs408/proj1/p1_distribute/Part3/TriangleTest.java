import static org.junit.jupiter.api.Assertions.assertEquals;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

public class TriangleTest {

    @ParameterizedTest
    @CsvSource({
        "150, 200, 130, 0",   
        "120, 120, 100, 1",   
        "201, 201, 201, -2"   
    })
    public void testTriangle(int a, int b, int c, int expected) {
        assertEquals(expected, Triangle.triangle(a, b, c));
    }
}


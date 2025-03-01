import static org.junit.jupiter.api.Assertions.assertEquals;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvFileSource;

public class TriangleTest {

    @ParameterizedTest
    @CsvFileSource(resources = "robust.csv", numLinesToSkip = 1)
    public void testTriangle(int a, int b, int c, int expected) {
        assertEquals(expected, Triangle.triangle(a, b, c));
    }
}


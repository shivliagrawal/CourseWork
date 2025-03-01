import static org.junit.jupiter.api.Assertions.assertEquals;
import org.junit.jupiter.api.Test;

public class TriangleTest {

    @Test
    public void testTriangleCase() {
        assertEquals(Triangle.SCALENE, Triangle.triangle(150, 200, 130));
    }
}


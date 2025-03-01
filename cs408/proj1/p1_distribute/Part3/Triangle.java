public class Triangle {
	public static final int OUT_OF_RANGE = -2;
	public static final int INVALID = -1;
	public static final int SCALENE = 0;
	public static final int ISOSELES = 1;
	public static final int EQUILATERAL = 2;

	public static int triangle(int a, int b, int c) {

		boolean c1, c2, c3, isATriangle;

		// Step 1: Validate Input
		c1 = (1 <= a) && (a <= 200);
		c2 = (1 <= b) && (b <= 200);
		c3 = (1 <= c) && (c <= 200);

		int triangleType = INVALID;
		if (!c1 || !c2 || !c3)
			triangleType = INVALID;
		else {
			// Step 2: Is A Triangle?
			if ((a < b + c) && (b < a + c) && (c < a + b))
				isATriangle = true;
			else
				isATriangle = false;	

			// Step 3: Determine Triangle Type
			if (isATriangle) {
				if ((a == b) && (b == c))
					triangleType = EQUILATERAL;
				else if ((a != b) && (a != c) && (b != c))
					triangleType = SCALENE;
				else
					triangleType = ISOSELES;
			} else
				triangleType = INVALID;
		}
		return triangleType;
	}
}

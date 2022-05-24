import java.util.HashSet;

class Test {
	static final int NUM_TO_TEST = 50000000;
	public static void main(String[] args) {
		int numToTest = Integer.parseInt(args[0]);
		HashSet<Integer> set = new HashSet<Integer>();
		for (int i = 0; i < numToTest; i++) {
			set.add(i);
		}
	}
}

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.util.Random;

import javax.swing.JFrame;
import javax.swing.JPanel;

@SuppressWarnings("serial")
public class Vis extends JPanel {
	public static final int num = 64;
	public static final int scanLength = 8;

	static int[] r = new int[num];
	static int[] g = new int[num];
	static int[] b = new int[num];
	static JFrame frame = new JFrame("Lux Testing");

	public static void main(String[] args) {
		frame.setBackground(Color.BLACK);
		frame.add(new Vis());
		frame.setSize(300, 300);
		frame.setVisible(true);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		reset();

		// do visualizations

		Random rand = new Random();
		int[] rn = new int[num];
		int[] gn = new int[num];
		int[] bn = new int[num];

		while (true) {
			for (int j = 0; j < scanLength; j++) {
				for (int i = 0; i < scanLength; i++) {
					if (rand.nextBoolean()) {
						rn[j * 8 + i] = 255;
					}
				}
			}

			r = rn;
			b = bn;
			g = gn;

			frame.repaint();
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			reset();
		}
	}

	public static void reset() {
		for (int i = 0; i < num; i++) {
			r[i] = 0;
			b[i] = 0;
			g[i] = 0;
		}
		frame.repaint();
	}

	@Override
	public void paint(Graphics graph) {
		Graphics2D g2d = (Graphics2D) graph;
		for (int i = 0; i < num; i++) {
			int rgb = (r[i] << 16) + (g[i] << 8) + b[i];
			g2d.setColor(new Color(rgb));
			int colOffset = (i / 8) * 30;
			int rowOffset = (i % 8) * 30;
			// System.out.println(colOffset + "\t" + rowOffset);
			g2d.fillOval(25 + colOffset, 15 + rowOffset, 30, 30);
		}
	}
}

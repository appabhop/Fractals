#include <SFML/Graphics.hpp>
#include <complex>
#include <thread>
#include <iostream>
#include <cmath>

sf::Image img;

int width = 1600;
int height = 1240;
int max_iterations = 2048;
int supersampling = 1, supersamplingSq = supersampling * supersampling;
double zoom = 0.004;

sf::Vector2f position(-0.5, 0);

enum sets : int {
	MANDELBROT,
	JULIA,
	BURNING_SHIP
};

struct colour {
	int r, g, b;
};

colour getColour(int iteration, double zx, double zy)
{
	iteration = pow((double)iteration / (double)max_iterations, 0.75) * (double)max_iterations; // for lighter image
	float modulus = sqrt(zx*zx + zy * zy);
	float mu = iteration - (log(log(modulus))) / log(2.f);
	float t = mu / max_iterations;
	float t2 = t * t;
	colour c;
	c.r = (9 * (1 - t) * t2 * t * 255);
	c.g = (15 * (1 - t) * (1 - t) * t2 * 255);
	c.b = (8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
	return c;
}

void mandelbrot(int threads, int start)
{
	for (int i = (start - 1) * (width / threads); i < start * (width / threads) * supersampling; i += supersampling)
	{
		for (int j = 0; j < height * supersampling; j += supersampling)
		{
			int r = 0, g = 0, b = 0;
			for (int i2 = 0; i2 < supersampling; i2++)
			{
				for (int j2 = 0; j2 < supersampling; j2++)
				{
					int iteration = 0;
					double x = (i + i2 - width / 2.0) * zoom + position.x;
					double y = (j + j2 - height / 2.0) * zoom + position.y;
					double zx = x;
					double zy = y;
					double xt;

					while (iteration++ < max_iterations && zx * zx + zy * zy < 4.0)
					{
						xt = zx * zx - zy * zy + x;
						zy = (2 * zx * zy) + y;
						zx = xt;
					}

					if (iteration < max_iterations)
					{
						colour c = getColour(iteration, zx, zy);
						r += c.r; g += c.g; b += c.b;
					}
				}
			}

			r /= supersamplingSq;
			g /= supersamplingSq;
			b /= supersamplingSq;

			img.setPixel(i / supersampling, j / supersampling, sf::Color(b, g, r));
		}
	}
}

void burning_ship(int threads, int start)
{
	for (int i = (start - 1) * (width / threads); i < start * (width / threads) * supersampling; i += supersampling)
	{
		for (int j = 0; j < height * supersampling; j += supersampling)
		{
			int r = 0, g = 0, b = 0;
			for (int i2 = 0; i2 < supersampling; i2++)
			{
				for (int j2 = 0; j2 < supersampling; j2++)
				{
					int iteration = 0;
					double x = (i + i2 - width / 2.0) * zoom + position.x;
					double y = (j + j2 - height / 2.0) * zoom + position.y;
					double zx = x;
					double zy = y;
					double xt;

					while (iteration++ < max_iterations && zx * zx + zy * zy < 4.0)
					{
						xt = zx * zx - zy * zy + x;
						zy = fabs((2 * zx * zy) + y);
						zx = fabs(xt);
					}

					if (iteration < max_iterations)
					{
						colour c = getColour(iteration, zx, zy);
						r += c.r; g += c.g; b += c.b;
					}
				}
			}

			r /= supersamplingSq;
			g /= supersamplingSq;
			b /= supersamplingSq;

			img.setPixel(i / supersampling, j / supersampling, sf::Color(b, g, r));
		}
	}
}

double scale(float min, float max, float high, float low, float value)
{
	return ((value - low) / (high - low)) * (max - min) + min;
}

void julia(int threads, int start)
{
	double cx = -0.7; // -0.4, 0.4
	double cy = 0.27015; // 0.6
	for (int i = (start - 1) * (width / threads); i < start * (width / threads) * supersampling; i += supersampling)
	{
		for (int j = 0; j < height * supersampling; j += supersampling)
		{
			int r = 0, g = 0, b = 0;
			for (int i2 = 0; i2 < supersampling; i2++)
			{
				for (int j2 = 0; j2 < supersampling; j2++)
				{
					int iteration = 0;
					double zx = (i + i2 - width / 2.0) * zoom + position.x;
					double zy = (j + j2 - height / 2.0) * zoom + position.y;
					double xt;

					while (iteration++ < max_iterations && zx * zx + zy * zy < 4.0)
					{
						xt = zx * zx - zy * zy;
						zy = 2 * zx * zy + cy;
						zx = xt + cx;
					}

					if (iteration < max_iterations)
					{
						if (iteration < max_iterations)
						{
							colour c = getColour(iteration, zx, zy);
							r += c.r; g += c.g; b += c.b;
						}
					}
				}
			}

			r /= supersamplingSq;
			g /= supersamplingSq;
			b /= supersamplingSq;

			img.setPixel(i / supersampling, j / supersampling, sf::Color(b, g, r));
		}
	}
}

void chooseSet(int set, int threadCount, int start)
{
	switch (set)
	{
	case MANDELBROT:
		mandelbrot(threadCount, start);
		break;
	case JULIA:
		julia(threadCount, start);
		break;
	case BURNING_SHIP:
		burning_ship(threadCount, start);
		break;
	}
}

void generateSet(int set, int threadCount)
{
	img.create(width, height, sf::Color::Black);
	std::vector<std::thread> threads;
	for (int i = 0; i < threadCount; i++)
	{
		threads.push_back(std::thread(&chooseSet, set, threadCount, i + 1));
	}
	for (std::thread& t : threads)
	{
		t.join();
	}
}

int main()
{
	img.create(width, height, sf::Color::Black);

	std::complex<double> z, c;
	int iteration = 0;

	int threadCount = std::thread::hardware_concurrency(); 

	int currentSet = MANDELBROT;
	generateSet(currentSet, threadCount);

	sf::Texture texture;
	texture.loadFromImage(img);
	sf::Sprite set;
	set.setTexture(texture);

	sf::RenderWindow window(sf::VideoMode(width, height), "Fractals");
	window.setFramerateLimit(60);

	bool update = false;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::KeyPressed)
			{
				int key = event.key.code;
				if (key == sf::Keyboard::W)
				{
					position.y -= zoom * 10;
					update = true;
				}
				else if (key == sf::Keyboard::A)
				{
					position.x -= zoom * 10;
					update = true;
				}
				else if (key == sf::Keyboard::S)
				{
					position.y += zoom * 10;
					update = true;
				}
				else if (key == sf::Keyboard::D)
				{
					position.x += zoom * 10;
					update = true;
				}
			}
			else if (event.type == sf::Event::MouseWheelScrolled)
			{
				if (event.mouseWheelScroll.delta < 0)
					zoom *= 1.1;
				else
					zoom *= 0.9;
				update = true;
			}
		}

		if (update == true)
		{
			generateSet(currentSet, threadCount);
			texture.loadFromImage(img);
			set.setTexture(texture);
			update = false;
		}

		window.clear();
		window.draw(set);
		window.display();
	}
}

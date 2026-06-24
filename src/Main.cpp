#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>

constexpr unsigned short TPS = 60;

/*
const std::vector<std::pair<sf::Vector2f, sf::Vector2f>> shapeData = {
	{{60.f, 100.f},  {180.f, 25.f}},
	{{120.f, 180.f}, {25.f, 200.f}},
	{{500.f, 120.f}, {250.f, 30.f}},
	{{650.f, 180.f}, {30.f, 180.f}},
	{{80.f, 650.f},  {200.f, 25.f}},
	{{300.f, 520.f}, {25.f, 150.f}},
	{{500.f, 620.f}, {220.f, 25.f}},
	{{520.f, 300.f}, {25.f, 140.f}},
	{{100.f, 450.f}, {140.f, 25.f}},
	{{700.f, 500.f}, {25.f, 120.f}}
};
*/

const std::vector<std::pair<sf::Vector2f, sf::Vector2f>> shapeData = {
	{{60.f, 100.f},  {180.f, 25.f}},
	{{120.f, 180.f}, {25.f, 200.f}},
	{{500.f, 120.f}, {250.f, 30.f}}
};

float cross(sf::Vector2f a, sf::Vector2f b) {
	return a.x * b.y - a.y * b.x;
}

float orient(sf::Vector2f a, sf::Vector2f b, sf::Vector2f c) {
	return cross({b.x - a.x, b.y - a.y}, {c.x - a.x, c.y - a.y});
}

// check if line segments AB and CD intersect
// TODO: deal with colinear / parallel lines
bool intersects(sf::Vector2f a, sf::Vector2f b, sf::Vector2f c, sf::Vector2f d) {
	float o1 = orient(a, b, c);
	float o2 = orient(a, b, d);
	float o3 = orient(c, d, a);
	float o4 = orient(c, d, b);

	return (o1 * o2 < 0) && (o3 * o4 < 0);
}

std::vector<std::pair<sf::Vector2f, sf::Vector2f>> decomposeShapes(const std::vector<std::unique_ptr<sf::Shape>>& shapes) {
	std::vector<std::pair<sf::Vector2f, sf::Vector2f>> lineSegments;
	
	for (const auto& shape : shapes) {
		for (std::size_t i = 0; i < shape->getPointCount() - 1; i++)
			lineSegments.push_back(std::pair(shape->getPoint(i) + shape->getPosition(), shape->getPoint(i + 1) + shape->getPosition()));

		lineSegments.push_back(std::pair(shape->getPoint(shape->getPointCount() - 1) + shape->getPosition(), shape->getPoint(0) + shape->getPosition()));
	}

	return lineSegments;
}

void raycast(std::vector<std::uint8_t>& pixels, const std::vector<std::unique_ptr<sf::Shape>>& shapes, const sf::Vector2f& sourcePosition) {
	std::vector<std::pair<sf::Vector2f, sf::Vector2f>> lineSegments = decomposeShapes(shapes);

	for (std::size_t y = 0; y < 800; y++) {
		for (std::size_t x = 0; x < 800; x++) {
			bool visible = true;
			for (const auto& line : lineSegments) {
				if (intersects(line.first, line.second, {static_cast<float>(x), static_cast<float>(y)}, sourcePosition)) {
					visible = false;
					break;
				}
			}

			std::size_t index = (y * 800 + x) * 4;
			pixels[index] = (visible ? 128 : 64);
			pixels[index + 1] = (visible ? 128 : 64);
			pixels[index + 2] = (visible ? 128 : 64);
			pixels[index + 3] = 255;
		}
	}
}

int main() {
	sf::CircleShape source;
	source.setPosition({395.f, 395.f});
	source.setRadius(10.f);
	source.setFillColor(sf::Color::Red);

	std::vector<std::unique_ptr<sf::Shape>> shapes;
	for (const auto& data : shapeData) {
		auto rs = std::make_unique<sf::RectangleShape>(data.second);
		rs->setFillColor(sf::Color::Black);
		rs->setPosition(data.first);
		shapes.push_back(std::move(rs));
	}

	//auto c = std::make_unique<sf::CircleShape>();
	//c->setPosition({385.f, 700.f});
	//c->setRadius(30.f);
	//c->setFillColor(sf::Color::Black);
	//shapes.push_back(std::move(c));

	std::vector<std::uint8_t> pixels(800 * 800 * 4);
	raycast(pixels, shapes, source.getPosition() + source.getGeometricCenter());

	sf::Texture texture(sf::Vector2u(800, 800));
	texture.update(pixels.data());

	sf::Sprite sprite(texture);
	sprite.setPosition({0.f, 0.f});

	sf::ContextSettings cs;
	cs.antiAliasingLevel = 0;

	sf::RenderWindow window(sf::VideoMode({ 800, 800 }), "Raycasting", sf::Style::Titlebar | sf::Style::Close, sf::State::Windowed, cs);

	sf::Clock clock;
	sf::Time accumulator;

	int frameCount = 0;
	int tickCount = 0;

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}
		}

		while (accumulator > sf::seconds(1.f / TPS)) {
			accumulator -= sf::seconds(1.f / TPS);

			// tick
			raycast(pixels, shapes, source.getPosition() + source.getGeometricCenter());
			texture.update(pixels.data());

			tickCount++;
			if (tickCount > TPS) {
				tickCount = 0;
				std::cout << "FPS: " << frameCount << std::endl;
				frameCount = 0;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
				source.setPosition(source.getPosition() + sf::Vector2f(0.f, -2.f));
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
				source.setPosition(source.getPosition() + sf::Vector2f(-2.f, 0.f));
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
				source.setPosition(source.getPosition() + sf::Vector2f(0.f, 2.f));
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
				source.setPosition(source.getPosition() + sf::Vector2f(2.f, 0.f));
		}

		// render
		window.clear(sf::Color(128, 128, 128));

		window.draw(sprite);
		window.draw(source);
		for (const auto& shape : shapes)
			window.draw(*shape);

		window.display();

		frameCount++;

		accumulator += clock.restart();
	}

	return 0;
}
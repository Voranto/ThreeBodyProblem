#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFML/Audio.hpp>
#include <cmath>
using namespace std;
using namespace sf;

const int frameheight = 800;
const int framewidth = 1200;
const int maxtrailsize = 255;
const float barwidth = 200;
int delay = 0;

bool velocity_arrow = false;

RenderWindow window;



static Vector2f multiplicacionvector(Vector2f vector, float num) {
	return Vector2f(vector.x * num, vector.y * num);
}

//clase planeta
class Planet {
public:
	Vector2f position;
	Vector2f velocity;
	float mass;
	Vector2f force;
	Vector2f acceleration;
	Color color;
	vector<Vector2f> trail;
	VertexArray velocity_line = VertexArray(Lines,2);
	CircleShape velocity_circle = CircleShape(3);

	Planet(Vector2f pos, Vector2f vel, float peso, Color col) : position(pos), velocity(vel), mass(peso), color(col) {}


	void calculateforce(vector<Planet> otherplanets) {
		force = Vector2f(0, 0);
		float constant = 5000000;
		float softening = 20.0f;  // Increase softening factor
		float maxForce = 1000.0f;  // Maximum allowable force to prevent spikes

		for (auto& obj : otherplanets) {
			if (&obj == this) continue;  // Skip self-force calculation

			// Vector distance
			Vector2f dist = position - obj.position;
			float module_dist = sqrt(dist.x * dist.x + dist.y * dist.y);

			if (module_dist != 0) {
				Vector2f normalized_dist = dist / module_dist;

				// Apply the softening factor and calculate force
				float f_module = (mass * obj.mass * constant) / ((module_dist * module_dist) + (softening * softening));

				// Cap the force to prevent spikes
				if (f_module > maxForce) {
					f_module = maxForce;
				}

				Vector2f F = f_module * normalized_dist;

				force -= F;
			}
		}

		acceleration = force / mass;
	}

	void adjustcamera(Vector2f centerpoint) {
		Vector2f center = Vector2f(400, 400);
		
		Vector2f vectorcentro = centerpoint - center;
		position -= vectorcentro;
		
	}

	void updatepos(float dt) {
		velocity += acceleration * dt;
		position += velocity * dt;
	}
};


//hacer scrollwheels que ajusten su masa
class scrollwheel {
public:
	Vector2f position;
	float ballpos;
	Color color;
	float start;
	float end;
	float ballradius = 10;

	scrollwheel(Vector2f pos, float start, float end, Color col) : position(pos),ballpos(pos.x), start(start), end(end), color(col) {}


};

//random function
int Random(int min, int max) {
	int val = min + rand() % max;
	return val;
}

vector<Planet> loadplanets(int numberofplanets) {
	vector<Planet> planets2;
	for (int i = 0; i < numberofplanets; i++) {
		//Random(300,frameheight-300)
		//Random(0, 255)
		planets2.insert(planets2.begin(),Planet(Vector2f(Random(200, frameheight - 200), Random(200, frameheight - 200)), Vector2f(10,10), 10, Color(Random(0, 255), Random(0, 255), Random(0, 255), 255)));
	}

	return planets2;
}





int main() {
	
	


	Clock clock;
	vector<Planet> planets;
	vector<scrollwheel> scrollwheels;

	int numberofplanets;
	cout << "How many planets do you desire: ";
	cin >> numberofplanets;

	planets = loadplanets(numberofplanets);

	for (int i = 0; i < planets.size(); i++) {
		scrollwheels.emplace_back(scrollwheel(Vector2f(900, 120 + ((frameheight) / planets.size()) * i), 5, 30, planets[i].color + Color(100, 100, 100, 0)));
	}

	scrollwheel initial = scrollwheel(Vector2f(900, 100 + ((frameheight) / planets.size())), 5, 100, Color(100, 100, 100, 0));
	scrollwheel* dragging = &initial;

	RenderWindow window(VideoMode(framewidth, frameheight), "Three Body Problem", Style::Close | Style::Titlebar);
	window.setFramerateLimit(240);

	//adding the font
	Font font;
	if (!font.loadFromFile("C:\\Windows\\Fonts\\impact.ttf")) {
		cerr << "Error loading font\n";
		return -1;
	}

	Text freeze_text;
	freeze_text.setFont(font);
	freeze_text.setString("FREEZE");
	freeze_text.setCharacterSize(80);
	freeze_text.setLetterSpacing(2);
	freeze_text.setPosition(frameheight + 5 , 5);
	freeze_text.setFillColor(Color(50, 50, 50, 255));



	Text reset_text;
	reset_text.setFont(font);
	reset_text.setString("RESET");
	reset_text.setCharacterSize(60);
	reset_text.setPosition(frameheight - 180, 10);
	reset_text.setFillColor(Color(0, 0, 0, 255));

	Planet closestplanet(Vector2f(-1000, -1000), Vector2f(0, 0), 0, Color(0, 0, 0, 0));
	Planet* closestplanetptr = &closestplanet;
	Planet* velocityptr = &closestplanet;
	



	Event event;
	

	bool mouse_click = false;
	bool frozen = false;

	//dibujos estaticos
	VertexArray lineadivisoria(Lines, 2);
	lineadivisoria[0] = Vector2f(frameheight, 0);
	lineadivisoria[1] = Vector2f(frameheight, frameheight);

	VertexArray lineadivisoriafreeze(Lines, 2);
	lineadivisoriafreeze[0] = Vector2f(frameheight, 100);
	lineadivisoriafreeze[1] = Vector2f(framewidth, 100);

	RectangleShape freezebutton(Vector2f(framewidth - frameheight, 100));
	freezebutton.setPosition(Vector2f(frameheight,0));
	freezebutton.setOutlineColor(Color(100, 100, 100, 255));
	freezebutton.setFillColor(Color(150, 150, 150, 255));


	RectangleShape resetbutton(Vector2f(200, 80));
	resetbutton.setPosition(Vector2f(frameheight-205, 5));
	resetbutton.setOutlineColor(Color(60, 100, 100, 255));
	resetbutton.setOutlineThickness(2);
	resetbutton.setFillColor(Color(180, 0, 0, 255));
	
	while (window.isOpen()) {

		//mouse position
		Vector2f mPos(Mouse::getPosition(window).x, Mouse::getPosition(window).y);
		window.clear();


		//draw dibujos estaticos
		window.draw(lineadivisoria);
		window.draw(lineadivisoriafreeze);
		window.draw(freezebutton);
		window.draw(freeze_text);
		window.draw(resetbutton);
		window.draw(reset_text);


		float deltaTime = clock.restart().asSeconds();
		if (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}

			//cuando se haga click con boton izquierdo
			if (event.type == sf::Event::MouseButtonPressed) //Mouse button Pressed
			{
				if (event.mouseButton.button == sf::Mouse::Left) {
					mouse_click = true;
					//check if im interacting with scrollwheels
					for (auto& obj : scrollwheels) {
						Vector2f dist = Vector2f(obj.ballpos, obj.position.y) - mPos;

						float dist2 = dist.x * dist.x + dist.y * dist.y;
						float buffer = 100;
						if (dist2 < obj.ballradius * obj.ballradius + buffer) {
							dragging = &obj;
						}
					}


					//check nearest planet for dragging
					float mindist2 = 999999999999999999;
					for (auto& obj : planets) {
						Vector2f dist = obj.position - mPos;
						if (dist.x * dist.x + dist.y * dist.y < mindist2) {
							mindist2 = dist.x * dist.x + dist.y * dist.y;
							closestplanetptr = &obj;
						}
					}

					//check if frozen button clicked
					if (mPos.x > frameheight and mPos.y < 100 and delay == 0) {
						delay = 10;
						if (frozen == false) {
							frozen = true;
							freeze_text.setString("UNFREEZE");
						}
						else { 
							frozen = false;
							freeze_text.setString("FREEZE");

							for (auto& obj : planets) {
								if (obj.velocity_line.getBounds().getPosition().x != 0 and obj.velocity_line.getBounds().getPosition().y != 0) {
									Vector2f pos1(obj.velocity_line[0].position.x , obj.velocity_line[0].position.y);
									Vector2f pos2(obj.velocity_line[1].position.x, obj.velocity_line[1].position.y);
									obj.velocity = multiplicacionvector(pos1 - pos2,5);
								}
								obj.velocity_line[0].position = Vector2f(0,0);
								obj.velocity_line[1].position = Vector2f(0, 0);
								obj.velocity_circle.setPosition(Vector2f(0, 0));
								obj.velocity_circle.setRadius(0);

							}
						}
					}
					if (mPos.x > frameheight - 205 and mPos.x < frameheight - 5 and mPos.y < 85 and frozen == false) {
						planets.clear();
						scrollwheels.clear();


						planets = loadplanets(numberofplanets);

						for (int i = 0; i < planets.size(); i++) {
							scrollwheels.emplace_back(scrollwheel(Vector2f(900, 200 + ((frameheight) / planets.size()) * i), 5, 30, planets[i].color + Color(100, 100, 100, 0)));
						}

						scrollwheel initial = scrollwheel(Vector2f(900, 120 + ((frameheight) / planets.size())), 5, 100, Color(100, 100, 100, 0));
						scrollwheel* dragging = &initial;
					}

				}
				else if (event.mouseButton.button == sf::Mouse::Right) {
					if (frozen == true) {
						for (auto& obj : planets) {
							Vector2f dist = obj.position - mPos;

							float dist2 = dist.x * dist.x + dist.y * dist.y;
							if (dist2 < obj.mass * obj.mass) {
								velocity_arrow = true;
								velocityptr = &obj;
							}
						}
					}
				}
			}
			if (event.type == sf::Event::MouseButtonReleased) //Mouse button Pressed
			{
				if (event.mouseButton.button == sf::Mouse::Left) {
					mouse_click = false;
					dragging = &initial;
				}
				else if (event.mouseButton.button == sf::Mouse::Right) {
					velocity_arrow = false;
					velocityptr = &closestplanet;
				}
			}
			//ajustar delay boton
			if (delay > 0) {
				delay -= 1;
			}

		}
		


		CircleShape drawing;
		drawing.setFillColor(Color(255, 0, 0, 255));
		float centerx = 0;
		float centery = 0;
		Vector2f center(0, 0);
		
		for (auto& obj : planets) {
			//actualizar posicion
			center += obj.position;
			if (frozen == false) {
				obj.calculateforce(planets);
				obj.updatepos(deltaTime);
			}
			

			//comprobar si es el planeta mas cercano

			//dibujar el trail
			CircleShape traildrawing;
			traildrawing.setRadius(obj.mass / 4);
			int temp = 255;
			for (auto& trailelement : obj.trail) {
				traildrawing.setFillColor(Color(255, 255, 255, temp));
				temp -= 1;
				traildrawing.setPosition(trailelement - Vector2f(obj.mass / 4, obj.mass / 4));
				window.draw(traildrawing);
			}
		}
		//calcular el centro
		center.x /= planets.size();
		center.y /= planets.size();
		int cntr = 0;
		for (auto& obj : planets) {


			//ajustar camara
			if (frozen == false) {
				obj.adjustcamera(center);
			}
			obj.trail.insert(obj.trail.begin(), obj.position);
			//dibujar el objeto en si
			drawing.setRadius(obj.mass);
			drawing.setPosition(obj.position - Vector2f(obj.mass, obj.mass));
			drawing.setFillColor(obj.color);

			

			//ajustar trail si es demasiado largo
			if (obj.trail.size() > maxtrailsize) {
				obj.trail.pop_back();
			}

			//codigo para coger planetas
			if (&obj == closestplanetptr and mouse_click == true and frozen == true and mPos.x < frameheight) {
				
				obj.position = mPos;
				obj.velocity = Vector2f(0,0);
			}
			window.draw(drawing);
			window.draw(obj.velocity_line);
			window.draw(obj.velocity_circle);
		}
		for (int j = 0; j < scrollwheels.size(); j ++) {
			
			//actualizar si click
			if (&scrollwheels[j] == dragging and mouse_click == true) {
				if (mPos.x > scrollwheels[j].position.x and mPos.x < scrollwheels[j].position.x + barwidth) {
					scrollwheels[j].ballpos = mPos.x;

					float tamano = (scrollwheels[j].ballpos - scrollwheels[j].position.x) * ((scrollwheels[j].end - scrollwheels[j].start) / barwidth ) + scrollwheels[j].start;
					
					planets[j].mass = tamano;

				}
			}

			//dibujar
			RectangleShape barra(Vector2f(barwidth, 5));
			barra.setPosition(scrollwheels[j].position);
			barra.setFillColor(scrollwheels[j].color);
			window.draw(barra);
			CircleShape ball(scrollwheels[j].ballradius);
			ball.setFillColor(scrollwheels[j].color);
			ball.setPosition(Vector2f(scrollwheels[j].ballpos - scrollwheels[j].ballradius, scrollwheels[j].position.y - scrollwheels[j].ballradius + 2.5));
			window.draw(ball);
		}

		//dibujar vector velocidad
		if (frozen == true) {
			if (velocity_arrow == true) {
				velocityptr->velocity_line[0] = velocityptr->position;
				velocityptr->velocity_line[1] = mPos;
				velocityptr->velocity_circle.setPosition(Vector2f( mPos.x - 5 , mPos.y -5));
				velocityptr->velocity_circle.setRadius(5);
			}
		}



		window.display();
	}
}

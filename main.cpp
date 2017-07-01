#include <iostream>
#include <sys/ioctl.h>
#include "utfstring.hpp"
#include "cell_attributes.hpp"
#include "blurses.hpp"
#include <stack>
#include <memory>
#include "braille_buffer.hpp"
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace threed {
	struct vertex {
		glm::vec3 position;
		glm::vec4 color;
		glm::vec3 normal;

		vertex transform(const glm::mat4 &mat) const {
			return vertex(*this).transform(mat);
		}

		vertex& transform(const glm::mat4 &mat) {
			position = mat * glm::vec4(position, 1.0);
			// normal = mat * glm::vec4(normal, 1.0);
			return *this;
		}
	};

	struct triangle {
		triangle(vertex v0, vertex v1, vertex v2) : v0(v0), v1(v1), v2(v2) {}

		vertex v0;
		vertex v1;
		vertex v2;

		triangle transform(const glm::mat4 &mat) const {
			return triangle(*this).transform(mat);
		}

		triangle& transform(const glm::mat4 &mat) {
			v0.transform(mat);
			v1.transform(mat);
			v2.transform(mat);
			return *this;
		}
	};

	struct light {
		glm::vec3 ambient;
		float ambientIntensity;
		glm::vec3 diffuse;
		float diffuseIntensity;
		glm::vec3 direction;
	};

	struct edge {
		int x;
		glm::vec4 color;
		float z;
		glm::vec3 normal;
	};

	struct span {
		bool isValid() const {
			return edges.size() == 2;
		}

		void addEdge(edge e) {
			switch (edges.size()) {
				case 0:
					edges.push_back(e);
					break;
				case 1:
					if (e.x > left().x) {
						edges = {left(), e};
					} else {
						edges = {e, left()};
					}
					break;
				case 2:
					if (e.x < left().x) {
						edges = {e, right()};
					} else if (e.x > right().x) {
						edges = {left(), e};
					}
			}
		}

		edge left() const {
			return edges[0];
		}

		edge right() const {
			return edges[1];
		}

		std::vector<edge> edges;
	};

	struct matrices {
		matrices() {}
		matrices(const matrices &m) : projection(m.projection), view(m.view), model(m.model) {}
		matrices(glm::mat4 projection, glm::mat4 view) : projection(projection), view(view) { }
		matrices(glm::mat4 projection, glm::mat4 view, glm::mat4 model) : projection(projection), view(view), model(model) { }

		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model;

		glm::mat4 vp() const {
			return projection * view;
		}

		glm::mat4 mvp() const {
			return vp() * model;
		}
	};

	bool inViewport(const glm::vec3 &v, uint16_t width, uint16_t height) {
		return v.x >= 0 || v.x < width || v.y >= 0 || v.y < height;
	}

	bool inViewport(const vertex &v, uint16_t width, uint16_t height) {
		return inViewport(v.position, width, height);
	}

	bool inViewport(const triangle &t, uint16_t width, uint16_t height) {
		return 
			inViewport(t.v0, width, height) ||
			inViewport(t.v1, width, height) ||
			inViewport(t.v2, width, height);
	}

	void addEdge(std::vector<span> &spans, vertex v1, vertex v2, const int offset) {
		int ydiff = std::ceil(v2.position.y - 0.5) - std::ceil(v1.position.y - 0.5);

		if (ydiff == 0) {
			return;
		}

		if (ydiff < 0) {
			std::swap(v1, v2);
		}

		int ilen = std::abs(ydiff);
		float len = std::abs(ydiff);

		glm::vec3 position_step = (v2.position - v1.position) / len;
		glm::vec4 color_step = (v2.color - v1.color) / len;
		glm::vec3 normal_step = (v2.normal - v1.normal) / len;

		vertex pos(v1);
		pos.position += position_step / 2.0f;

		const int ystart = std::ceil(v1.position.y - 0.5);
		const int yend = std::ceil(v2.position.y - 0.5);

		for (int ypos = ystart; ypos < yend; ypos++) {
			int yp = ypos - offset;

			if (yp >= 0 && yp < spans.size()) {
				edge e;
				e.x = pos.position.x;
				e.z = pos.position.z;
				e.color = pos.color;
				e.normal = pos.normal;
				spans.at(yp).addEdge(e);
			}

			pos.position += position_step;
			pos.color += color_step;
			pos.normal += normal_step;
		}
	}

	template <typename T>
	T lerp(T a, T b, float t) {
		return a + (b - a) * t;
	}

	void draw(Display &display, std::vector<float> &depth_buffer, const std::vector<span> &spans, const int offset, light &l) {
		int y = offset;

		for (const span s : spans) {
			if (!s.isValid()) {
				y++;
				continue;
			}

			if (y < 0) {
				y++;
				continue;
			}


			edge edge1 = s.left();
			edge edge2 = s.right();

			float len = (edge2.x - edge1.x);

			for (int x = edge1.x; x < edge2.x; x++) {
				float pos = (x - edge1.x) / len;
				float z = lerp(edge1.z, edge2.z, pos);

				const size_t o = y * display.width() + x;

				if (o >= depth_buffer.size()) {
					continue;
				}

				if (depth_buffer[o] < z) {
					continue;
				}

				depth_buffer[o] = z;

				glm::vec4 c = lerp(edge1.color, edge2.color, pos);
				glm::vec3 n = lerp(edge1.normal, edge2.normal, pos);

				glm::vec3 fv = n * l.direction;
				float factor = glm::clamp(-1.0f * (fv.x + fv.y + fv.z), 0.0f, 1.0f);
				c = c * glm::vec4((l.ambient * l.ambientIntensity) + (factor * l.diffuse * l.diffuseIntensity), 1.0f);
				c.r = glm::clamp(c.r, 0.0f, 1.0f);
				c.g = glm::clamp(c.g, 0.0f, 1.0f);
				c.b = glm::clamp(c.b, 0.0f, 1.0f);

				Cell cell = display.attr().bg(Color(c.r * 255, c.g * 255, c.b * 255)).fg(0xffffff).buildCell();
				cell.data = " ";
				display.set(x, y, cell);
			}

			y++;
		}
	}

	void draw(Display &display, std::vector<float> &depth_buffer, triangle tri, matrices mat) {
		if (!inViewport(tri, display.width(), display.height())) {
			return;
		}

		tri.transform(mat.mvp());
		tri.transform(glm::translate(glm::mat4(1.0), glm::vec3(display.width() / 2.0, display.height() / 2.0, 0.0)));
		// tri.transform(glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 0.5f, 1.0f)));

		const std::pair<float, float> minmax = std::minmax({
			tri.v0.position.y,
			tri.v1.position.y,
			tri.v2.position.y
		});

		const int ymin = minmax.first;
		const int ymax = minmax.second;

		std::vector<span> spans(ymax - ymin);

		addEdge(spans, tri.v0, tri.v1, ymin);
		addEdge(spans, tri.v1, tri.v2, ymin);
		addEdge(spans, tri.v2, tri.v0, ymin);

		light l;
		l.ambient = glm::vec3(1.0, 1.0, 1.0);
		l.ambientIntensity = 0.2;
		l.diffuse = glm::vec3(1.0, 1.0, 1.0);
		l.diffuseIntensity = 0.8;
		l.direction = mat.model * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // mvp * glm::vec4(glm::vec3(0.0, 0.0, 1.0), 1.0);

		draw(display, depth_buffer, spans, ymin, l);
	}

	glm::mat4 mvp(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &model) {
		return projection * view * model;
	}

	vertex makeVertex(float x, float y, float z, float r, float g, float b, float nx = 0.0, float ny = 0.0, float nz = 0.0) {
		return {
			glm::vec3(x, y, z),
			glm::vec4(r, g, b, 1.0),
			glm::vec3(nx, ny, nz)
		};
	}

	void render(Display &display, unsigned long ticks) {
		std::vector<float> depth_buffer(display.width() * display.height(), std::numeric_limits<float>::infinity());

		matrices m;
		m.projection = glm::perspective(30.0f, 4.0f / 3.0f, 0.1f, 2000.0f);

		m.view = glm::lookAt(
			glm::vec3(0.0, 20.0, -20.0),
			glm::vec3(0.0, 0.0, 0.0),
			glm::vec3(0.0, 0.0, 1.0)
		);

		float x = 10.0;
		
		std::list<triangle> triangles = {
			{
				makeVertex( -x, -x,  x, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0),
				makeVertex( -x,  x,  x, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0),
				makeVertex(  x, -x,  x, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0)
			}, {
				makeVertex( -x,  x,  x, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0),
				makeVertex(  x, -x,  x, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0),
				makeVertex(  x,  x,  x, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0)
			}, {
				makeVertex( -x, -x, -x, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0),
				makeVertex(  x, -x, -x, 0.0, 1.0, 0.0, 0.0, 0.0, -1.0),
				makeVertex(  x,  x, -x, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0),
			}, {
				makeVertex( -x, -x, -x, 1.0, 1.0, 0.0, 0.0, 0.0, -1.0),
				makeVertex(  x,  x, -x, 0.0, 1.0, 1.0, 0.0, 0.0, -1.0),
				makeVertex( -x,  x, -x, 1.0, 0.0, 1.0, 0.0, 0.0, -1.0)
			}, {
				makeVertex( -x,  x, -x, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0),
				makeVertex( -x,  x,  x, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0),
				makeVertex(  x,  x, -x, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0)
			}, {
				makeVertex( -x,  x,  x, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0),
				makeVertex(  x,  x, -x, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0),
				makeVertex(  x,  x,  x, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0)
			}, {
				makeVertex( -x, -x, -x, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0),
				makeVertex(  x, -x, -x, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0),
				makeVertex( -x, -x,  x, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0)
			}, {
				makeVertex( -x, -x,  x, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0),
				makeVertex(  x, -x,  x, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0),
				makeVertex(  x, -x, -x, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0)
			}, {
				makeVertex(  x, -x, -x, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0),
				makeVertex(  x, -x,  x, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0),
				makeVertex(  x,  x, -x, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0)
			}, {
				makeVertex(  x, -x,  x, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0),
				makeVertex(  x,  x, -x, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0),
				makeVertex(  x,  x,  x, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0)
			}, {
				makeVertex( -x, -x, -x, 1.0, 1.0, 0.0, -0.557, -0.557, -0.557),
				makeVertex( -x,  x, -x, 1.0, 1.0, 0.0, -0.557,  0.557, -0.557),
				makeVertex( -x, -x,  x, 1.0, 1.0, 0.0, -0.557, -0.557,  0.557)
			}, {
				makeVertex( -x, -x,  x, 1.0, 1.0, 0.0, -0.557, -0.557,  0.557),
				makeVertex( -x,  x,  x, 1.0, 1.0, 0.0, -0.557,  0.557,  0.557),
				makeVertex( -x,  x, -x, 1.0, 1.0, 0.0, -0.557,  0.557, -0.557)
			}
		};

		glm::mat4 model(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		// model = glm::rotate(model, ticks / 1000.0f, glm::vec3(1.0, 1.0, 0.0));
		model = glm::rotate(model, ticks / 500.0f, glm::vec3(0.9, 0.75, 1.0));
		model = glm::scale(model, 1.0f + glm::vec3(std::sin(ticks / 750.0f) * 0.5f));
		m.model = model;

		for (const triangle &t : triangles) {
			draw(display, depth_buffer, t, m);
		}

		/*
		m.model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, -30.0f, 0.0f));
		m.model = glm::rotate(m.model, ticks / 500.0f, glm::vec3(0.5, 0.0, 1.0));
		m.model = glm::rotate(m.model, ticks / 1000.0f, glm::vec3(0.0, 1.0, 0.0));
		m.model = glm::scale(m.model, glm::vec3(1.2f));

		const int c = 10;
		const float square = c * c;

		for (int y = -c; y < c; y++) {
			for (int x = -c; x < c; x++) {
				for (int z = -c; z < c; z++) {
					const glm::vec3 point = m.mvp() * glm::vec4(x, y, z, 1.0);
					const Color color(x * x / square * 255, y * y / square * 255, z * z / square * 255);

					const size_t o = point.y * display.width() + point.x;

					if (point.z >= depth_buffer[o]) {
						continue;
					}

					depth_buffer[o] = point.z;
					display.set(point.x, point.y, display.attr().bg(color).buildCell());
				}
			}
		}
		*/
	}
}

class State {
	public:
		State() { }
		~State() { }

		virtual void handleKey(Display &display, const Key& key, unsigned long ticks) = 0;
		virtual void update(unsigned long ticks) = 0;
		virtual void draw(Display& display) = 0;
};

typedef std::shared_ptr<State> StatePtr;

class MainState : public State {
	void handleKey(Display &display, const Key& key, unsigned long ticks) {
	}

	void update(unsigned long ticks) {
		_t = ticks;
	}

	void draw(Display& display) {
		threed::render(display, _t);
	}

	unsigned long _t;
};

class Application {
	public:
		Application() {
			pushState(std::make_shared<MainState>());
		}

		void run() {
			Blurses::start([&](Display &display, std::list<Key> keys, unsigned long ticks) -> bool {
				for (const Key &key : keys) {
					currentState().handleKey(display, key, ticks);
				}

				currentState().update(ticks);
				currentState().draw(display);

				return true;
			});
		}

		void pushState(StatePtr state) {
			_states.push(state);
		}

		void popState() {
			_states.pop();
		}

	private:
		std::stack<StatePtr> _states;

		State& currentState() {
			return *_states.top();
		}
};

int main() {
	Application app;
	app.run();
	return 0;
}

#include "app.hpp"

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include <bitset>
#include <chrono>
#include <iostream>
#include <iterator>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include "config.hpp"
#include "util.hpp"
#if __has_include ("bruteforcer/bruteforcer.hpp")
#	define BRUTEFORCER_INCLUDED
#	include "bruteforcer/binding.hpp"
#	include "bruteforcer/bruteforcer.hpp"
	bool bruteforcerFoundPath = false;
	bool bruteforcerRunning = false;
	bruteforcer::board b;
	bruteforcer::board bclone;
	bruteforcer::bfstats bstats;
	std::thread bfthr;
	std::mutex bfappchatmut;
	app *bfappptr;
	std::mutex bftrailmut;
	std::vector<float> bftrail;
	bool bfmadetrail;
	bool bfautoSendPath = false;
	size_t bfPathLen = 0, bfcurrPath = 0;
	std::vector<std::vector<bruteforcer::packedmove>> bfpaths;
	size_t bfshownPath;

	void pathfound(bruteforcer::packedmove *mvs, size_t depth) {
		bruteforcerFoundPath = true;
		bfbinding::printpaths(mvs, depth);
		bfmadetrail = false;
		bftrailmut.lock();
		bfbinding::maketrail(bftrail, bclone, mvs, depth);
		bfmadetrail = true;
		bftrailmut.unlock();
		bfPathLen = depth;
		uint8_t mcounts[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		uint8_t mcounts2[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		for (size_t i = 0; i < depth; ++i) {
			++mcounts[mvs[i]];
		}
		for (const std::vector<bruteforcer::packedmove> &path : bfpaths) {
			for (size_t i = 0; i < 32; ++i) {
				mcounts2[i] = 0;
			}
			for (size_t i = 0; i < depth; ++i) {
				++mcounts2[path[i]];
			}
			if (std::equal(std::begin(mcounts), std::end(mcounts), std::begin(mcounts2))) {
				return; // is permutation
			}
		}
		bfpaths.emplace_back();
		bfpaths.back().assign(mvs, mvs + depth);
	}
	void runBruteforcer(target trg) {
		bfpaths.clear();
		bfshownPath = false;
		bfpaths.shrink_to_fit();
		bclone = b;
		bruteforcerFoundPath = false;
		bruteforcerRunning = true;
		bruteforcer::htable.clear();
		bruteforcer::htablemax.clear();
		bfbinding::precomp(b, trg);
		char row[64];
		sprintf(row, "[Bruteforce]: | %5s | %12s | %7s | %12s |",
			"depth", "nodes", "tr hr", "time");
		row[63] = '\0';
		bfappptr->writeChat(row);
		printf("[Bruteforcer]: | %5s | %12s | %12s | %12s | %12s | %12s | %7s | %12s |\n",
			"depth", "nodes", "leaf", "inner", "tr", "trmax", "tr hr", "time");
		for (size_t depth = 1; bruteforcerRunning; ++depth) {
			auto starttm = std::chrono::high_resolution_clock::now();
			bfbinding::search(depth, trg, b, &bruteforcerRunning, bstats, pathfound);
			auto endtm = std::chrono::high_resolution_clock::now();
			int64_t durus = std::chrono::duration_cast<std::chrono::microseconds>(endtm - starttm).count();
			float durms = durus * 0.001f;
			uint32_t nodes = bstats.nodes + bstats.leaf;
			float trHrPercent = static_cast<float>(bstats.trhits + bstats.trmaxhits) /
					static_cast<float>(bstats.nodes + bstats.trhits + bstats.trmaxhits) * 100.f;
			sprintf(row, "[Bruteforce]: | %5lu | %12u | %6.2f%% | %10.3fms |",
				depth, nodes, trHrPercent, durms);
			row[63] = '\0';
			bfappptr->writeChat(row);
			printf("[Bruteforcer]: | %5lu | %12u | %12u | %12u | %12u | %12u | %6.2f%% | %10.3fms |\n",
				depth, nodes, bstats.leaf, bstats.nodes, bstats.trhits, bstats.trmaxhits, trHrPercent, durms);
			if (bruteforcerFoundPath)
				break;
		}
		bfappptr->writeChat(std::string("[Bruteforcer]: found ") + std::to_string(bfpaths.size()) + " unique paths");
		std::cout << "[Bruteforcer]: found" << bfpaths.size() << " unique paths" << std::endl;
	}
#endif

void onRecv(packet &p, void *data) {
	((app *)data)->recv(p);
}

app::app(int ww, int wh, const char *title) : ww(ww), wh(wh), cl(onRecv, this), showtrail(false), camorient(1, 0), showtraildatadirty(false), showing(false), selectedbot(-1) {
	w = glfwCreateWindow(ww, wh, title, NULL, NULL);
	glfwMakeContextCurrent(w);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "OpenGL load failed. Does your graphics card support OpenGL 4.0 core?" << std::endl;
	}
	cfg.load();
	fov = 2.f;
	for (size_t i = 0; i < BOARD_TILES; ++i) {
		bool *tile = brd.walls[i % BOARD_SIZE][i / BOARD_SIZE];
		for (uint8_t j = 0; j < 4; ++j)
			tile[j] = false;
	}
	for (uint8_t i = 0; i < 5; ++i) {
		bots[i].color = static_cast<colors::color_t>(i);
		bots[i].pos = glm::ivec2(i, 0);
	}
	trg.color = colors::GREEN;
	trg.pos = glm::ivec2(12, 13);
	setSun();
	initRendering();
	resize(ww, wh);

	stg = gamestage::MENU;

#ifdef BRUTEFORCER_INCLUDED
	bruteforcer::initHashVals(time(NULL));
	bfappptr = this;
#endif
}
app::~app() {
#ifdef BRUTEFORCER_INCLUDED
	if (bruteforcerRunning) {
		bruteforcerRunning = false;
		bfthr.join();
	}
#endif
	if (cl.running) {
		cl.send(packet(packets::C_S_DISCONNECT, nullptr, 0));
		cl.disconnect();
	}
	glgui::terminate();
}
void app::mainloop() {
	double prev = glfwGetTime();
	while (!glfwWindowShouldClose(w)) {
		double now = glfwGetTime();
		dt = static_cast<float>(now - prev);
		prev = now;
		tick();
		glfwSwapBuffers(w);
		glfwPollEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(cfg.sleepms));
	}
}
void app::no_event_mainloop() {
	double prev = glfwGetTime();
	while (!glfwWindowShouldClose(w)) {
		double now = glfwGetTime();
		dt = static_cast<float>(now - prev);
		prev = now;
		tick();
		glfwSwapBuffers(w);
		std::this_thread::sleep_for(std::chrono::milliseconds(cfg.sleepms));
	}
}
void app::click(int btn, int act, int mod) {
	(void)mod;
	double mx, my;
	glfwGetCursorPos(w, &mx, &my);
	float x = static_cast<float>(mx), y = static_cast<float>(my);
	glgui::container *currgui = stg == gamestage::MENU ? &gui : &ingamegui;
	if (act == GLFW_PRESS) {
		if (btn == GLFW_MOUSE_BUTTON_LEFT)
			currgui->unfocus();
		currgui->mousedown(btn, x, y);
		if (btn == GLFW_MOUSE_BUTTON_LEFT) {
			clickfbo.bind();
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			int px = static_cast<int>(x), py = wh - static_cast<int>(y) - 1;
			if (cfg.antialias) {
				postfboms.blitTo(clickfbo, GL_STENCIL_BUFFER_BIT, GL_NEAREST, px, py, px+1, py+1, 0, 0, 1, 1);
			} else {
				postfbo.blitTo(clickfbo, GL_STENCIL_BUFFER_BIT, GL_NEAREST, px, py, px+1, py+1, 0, 0, 1, 1);
			}
			unsigned char pixel[4];
			clickfbo.bind();
			glReadPixels(0, 0, 1, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, pixel);
			if (pixel[0] > 0 && pixel[0] < 6) {
				selectedbot = pixel[0] - 1;
			} else if (pixel[0] > 5 && pixel[0] < 10) {
				unsigned char dat = (static_cast<unsigned char>(selectedbot) << 4) | (pixel[0] - 6);
				cl.send(packet(packets::C_S_MOVE, reinterpret_cast<char *>(&dat), 1));
			} else {
				selectedbot = -1;
			}
		}
	} else if (act == GLFW_RELEASE) {
		currgui->mouseup(btn, x, y);
	}
}
void app::scroll(int x, int y) {
	(void)x;
	fov -= cfg.scrollsensitivity * y;
	if (fov > 2.5f) fov = 2.5f;
	if (fov < 0.5f) fov = 0.5f;
	proj = glm::perspective(fov, static_cast<float>(ww) / wh, .1f, 100.f);
}
void app::key(int key, int act, int mod) {
	glgui::container *currgui = stg == gamestage::MENU ? &gui : &ingamegui;
	if (act == GLFW_PRESS) {
		currgui->keydown(key, mod);
	} else if (act == GLFW_RELEASE) {
		currgui->keyup(key, mod);
	}
}
void app::write(unsigned int c) {
	if (c > 127) {
		std::cout << "[Input]: can't process non ASCII characters." << std::endl;
		return;
	}
	char cc = static_cast<char>(c);
	glgui::container *currgui = stg == gamestage::MENU ? &gui : &ingamegui;
	currgui->keywrite(cc);
}
void app::resize(int ww, int wh) {
	this->ww = ww;
	this->wh = wh;
	proj = glm::perspective(fov, static_cast<float>(ww) / wh, .1f, 100.f);
	posttex.bind();
	posttex.size = glm::ivec2(ww, wh);
	posttex.upload(NULL, GL_RGBA, GL_RGBA16F, GL_FLOAT);
	posttexover.bind();
	posttexover.size = glm::ivec2(ww, wh);
	posttexover.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	postdepth.bind();
	postdepth.size = glm::ivec2(ww, wh);
	postdepth.upload(NULL, GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8);
	posttexms.bind();
	posttexms.size = glm::ivec2(ww, wh);
	posttexms.generate(GL_RGBA16F, cfg.antialiasSamples);
	posttexoverms.bind();
	posttexoverms.size = glm::ivec2(ww, wh);
	posttexoverms.generate(GL_RGBA8, cfg.antialiasSamples);
	postdepthms.bind();
	postdepthms.size = glm::ivec2(ww, wh);
	postdepthms.generate(GL_DEPTH24_STENCIL8, cfg.antialiasSamples);
	tmptex.bind();
	tmptex.size = glm::ivec2(ww, wh);
	tmptex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	tmp2tex.bind();
	tmp2tex.size = glm::ivec2(ww, wh);
	tmp2tex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	gui.size = glm::ivec2(ww, wh);
	gui.resize();
	ingamegui.size = glm::ivec2(ww, wh);
	ingamegui.resize();
}
void app::recv(const packet &p) {
	switch (p.type()) {
	case packets::S_C_DISCONNECT:
		std::cout << "Disconnected!" << std::endl;
		break;
	case packets::S_C_WALLS:{
		size_t x = 0, y = 0;
		for (size_t i = 0; i < p.size(); ++i) {
			char dat1(p.data()[i] >> 4 & 0xf);
			char dat2(p.data()[i] & 0xf);
			for (uint8_t j = 0; j < 4; ++j) {
				brd.walls[x][y][j] = dat1 & 1;
				brd.walls[x+1][y][j] = dat2 & 1;
				dat1 >>= 1;
				dat2 >>= 1;
			}
			x += 2;
			if (i % 8 == 7) {
				++y;
				x = 0;
			}
		}
		break;
	}
	case packets::S_C_ROBOTS:
		for (size_t i = 0; i < 5; ++i) {
			bots[i].startpos.x = p.data()[i] & 0xf;
			bots[i].startpos.y = p.data()[i] >> 4 & 0xf;
			bots[i].pos = bots[i].startpos;
		}
		break;
	case packets::S_C_TARGET:
		trg.pos.x = p.data()[0] & 0xf;
		trg.pos.y = p.data()[0] >> 4 & 0xf;
		trg.color = static_cast<colors::color_t>(p.data()[1] & 0b111);
		if (showtrail) {
			trail.del();
			trailvbo.del();
			showtrail = false;
		}
		showtraildatalock.lock();
		showtraildata.clear();
		showtraildatadirty = true;
		showtraildatalock.unlock();
		bestPath = 999;
		lbbestpath.setText("");
#ifdef BRUTEFORCER_INCLUDED
		bfcurrPath = 0;
#endif
		break;
	case packets::S_C_FOUND_PATH:{
		size_t path = static_cast<size_t>(static_cast<unsigned char>(p.data()[0]));
		std::string name = std::string(p.data(), p.size());
		std::string text("{" + name + "} found path with length " + std::to_string(path));
		writeChat(text);
		std::cout << "[FOUND PATH]: " << text << std::endl;
		if (path < bestPath) {
			bestPath = path;
			lbbestpath.setText(std::to_string(path) + " by " + name);
		}
		break;
	}
	case packets::S_C_TIMEOUT:{
		writeChat("Timeout!");
		std::cout << "[TIMEOUT]: timeout!" << std::endl;
		break;
	}
	case packets::S_C_MOVE:{
		float y = .0625f;
		unsigned char col = p.data()[1];
		unsigned char t = p.data()[0];
		glm::ivec2 from = bots[col].pos;
		glm::ivec2 to = glm::ivec2(t & 0xf, t >> 4);
		bots[col].pos = to;
		movestack.push(std::make_pair(static_cast<colors::color_t>(col), from));
		showtraildatalock.lock();
		showtraildata.push_back(from.x * .125f - 0.9375f);
		showtraildata.push_back(y);
		showtraildata.push_back(from.y * .125f - 0.9375f);
		showtraildata.push_back(colors::toRGB[col].r);
		showtraildata.push_back(colors::toRGB[col].g);
		showtraildata.push_back(colors::toRGB[col].b);
		showtraildata.push_back(0);
		showtraildata.push_back(0);
		y += 0.00025f;
		showtraildata.push_back(to.x * .125f - 0.9375f);
		showtraildata.push_back(y);
		showtraildata.push_back(to.y * .125f - 0.9375f);
		showtraildata.push_back(colors::toRGB[col].r);
		showtraildata.push_back(colors::toRGB[col].g);
		showtraildata.push_back(colors::toRGB[col].b);
		showtraildata.push_back(0);
		showtraildata.push_back(0);
		showtraildatadirty = true;
		showtraildatalock.unlock();
		break;
	}
	case packets::S_C_UNDO_MOVE:{
		bots[movestack.top().first].pos = movestack.top().second;
		movestack.pop();
		showtraildatalock.lock();
		for (size_t i = 0; i < 16; ++i)
			showtraildata.pop_back();
		showtraildatadirty = true;
		showtraildatalock.unlock();
		break;
	}
	case packets::S_C_ROBOT_RESET:{
		std::stack<std::pair<colors::color_t, glm::ivec2>> emptystack;
		movestack.swap(emptystack);
		for (uint8_t i = 0; i <= colors::GRAY; ++i) {
			bots[i].pos = bots[i].startpos;
		}
		showtraildatalock.lock();
		showtraildata.clear();
		showtraildatadirty = true;
		showtraildatalock.unlock();
		break;
	}
	case packets::S_C_START_SHOW:{
		std::stack<std::pair<colors::color_t, glm::ivec2>> emptystack;
		movestack.swap(emptystack);
		for (uint8_t i = 0; i <= colors::GRAY; ++i) {
			bots[i].pos = bots[i].startpos;
		}
		showtraildatalock.lock();
		showtraildata.clear();
		showtraildatadirty = true;
		showtraildatalock.unlock();
		writeChat("You are showing!");
		showing = true;
		break;
	}
	case packets::S_C_STOP_SHOW:{
		std::stack<std::pair<colors::color_t, glm::ivec2>> emptystack;
		movestack.swap(emptystack);
		for (uint8_t i = 0; i <= colors::GRAY; ++i) {
			bots[i].pos = bots[i].startpos;
		}
		showtraildatalock.lock();
		showtraildata.clear();
		showtraildatadirty = true;
		showtraildatalock.unlock();
		writeChat("You aren't showing anymore!");
		showing = false;
		break;
	}
	case packets::S_C_SHOW:{
		std::stack<std::pair<colors::color_t, glm::ivec2>> emptystack;
		movestack.swap(emptystack);
		for (uint8_t i = 0; i <= colors::GRAY; ++i) {
			bots[i].pos = bots[i].startpos;
		}
		showtraildatalock.lock();
		showtraildata.clear();
		showtraildatadirty = true;
		showtraildatalock.unlock();
		writeChat(std::string(p.data(), p.size()) + " is showing!");
		break;
	}
	case packets::S_C_POINT:{
		std::stack<std::pair<colors::color_t, glm::ivec2>> emptystack;
		movestack.swap(emptystack);
		for (uint8_t i = 0; i <= colors::GRAY; ++i) {
			bots[i].pos = bots[i].startpos;
		}
		showtraildatalock.lock();
		showtraildata.clear();
		showtraildatadirty = true;
		showtraildatalock.unlock();
		writeChat(std::string(p.data(), p.size()) + " got a point!");
		break;
	}
	case packets::S_C_GAME_OVER:{
		writeChat("Game over!");
		writeChat(std::string(p.data(), p.size()) + " won!");
		break;
	}
	case packets::S_C_MESSAGE:{
		std::string msg(p.data(), p.size());
		writeChat(msg);
		std::cout << "[Chat]: " << msg << std::endl;
		break;
	}
	default:
		break;
	}
}
void app::connect() {
	std::cout << "[Networking]: connecting to " << tbip.text << ":" << tbport.text << std::endl;
	try {
		cl.connect(tbip.text, tbport.text);
		stg = gamestage::IN_GAME;
		cl.send(packet(packets::C_S_NICKNAME, tbnick.text.c_str(), tbnick.text.size()));
	} catch (const std::system_error &e) {
		std::cout << "[Networking]: failed connecting to " << tbip.text << ":" << tbport.text
			<< " " << e.what() << std::endl;
	}
}
void app::tbgameWrite() {
	if (tbgame.text.size() > 0) {
		if (tbgame.text[0] == '+') { // guess marker
			std::string numstr = tbgame.text.substr(1);
			auto it = std::find_if(numstr.begin(), numstr.end(), [](const char &c){ return !isdigit(c); });
			if (it == numstr.end()) {
				unsigned char pathlen = static_cast<unsigned char>(std::stoi(numstr));
				packet p(packets::C_S_FOUND_PATH, reinterpret_cast<char *>(&pathlen), 1);
				cl.send(p);
			} else {
				writeChat(std::string("\"") + numstr + "\" is not a number!");
				std::cout << "[ChatInput]: \"" << numstr << "\" is not a number!" << std::endl;
			}
		} else if (tbgame.text[0] == '!') { // command marker
			std::string cmdstr = tbgame.text.substr(1);
			std::stringstream cmdss(cmdstr);
			std::string cmd, arg;
			cmdss >> std::ws;
			cmdss >> cmd;
			if (cmd == "bruteforce" || cmd == "bf") {
#ifdef BRUTEFORCER_INCLUDED
				cmdss >> arg;
				if (arg == "") {
					arg = bruteforcerRunning ? "stop" : "run";
				}
				if (arg == "run") {
					writeChat("[Bruteforcer]: Starting search");
					std::cout << "[Bruteforcer]: Starting search" << std::endl;
					b.tcol = static_cast<bruteforcer::bcolors::color_t>(trg.color);
					bfbinding::translateboard(brd, bots, b);
					bfthr = std::thread(runBruteforcer, trg);
				} else if (arg == "stop") {
					writeChat("[Bruteforcer]: Stopping search");
					std::cout << "[Bruteforcer]: Stopping search" << std::endl;
					bruteforcerRunning = false;
					bfthr.join();
				} else if (arg == "autosend") {
					bfautoSendPath = true;
				} else if (arg == "autosendstop") {
					bfautoSendPath = false;
				} else if (arg == "prevpath" || arg == "p") {
					bfmadetrail = false;
					bftrailmut.lock();
					if (bfshownPath > 0) {
						--bfshownPath;
					}
					bfbinding::maketrail(bftrail, bclone, bfpaths[bfshownPath].data(), bfpaths[bfshownPath].size());
					bfmadetrail = true;
					bftrailmut.unlock();
				} else if (arg == "nextpath" || arg == "n") {
					bfmadetrail = false;
					bftrailmut.lock();
					if (bfshownPath < bfpaths.size() - 1) {
						++bfshownPath;
					}
					bfbinding::maketrail(bftrail, bclone, bfpaths[bfshownPath].data(), bfpaths[bfshownPath].size());
					bfmadetrail = true;
					bftrailmut.unlock();
				} else if (arg == "pathcount" || arg == "pathc") {
					writeChat(std::string("[Bruteforcer]: found ") + std::to_string(bfpaths.size()) + " paths");
					std::cout << "[Bruteforcer]: found" << bfpaths.size() << " paths" << std::endl;
				}
#else
				writeChat("[Bruteforcer]: Bruteforcer not included in this build.");
				std::cout << "[Bruteforcer]: Bruteforcer not included in this build." << std::endl;
#endif
			} else if (cmd == "renick") {
				cmdss >> arg;
				cl.send(packet(packets::C_S_NICKNAME, arg.c_str(), arg.size()));
			} else if (cmd == "clear") {
				lbchat.setText("");
			} else if (cmd == "deletetrail") {
				if (showtrail) {
					trail.del();
					trailvbo.del();
					showtrail = false;
				}
			} else if (cmd == "settrg") {
				static std::map<std::string, colors::color_t> str2col{
					{"red",colors::RED},
					{"green",colors::GREEN},
					{"blue",colors::BLUE},
					{"yellow",colors::YELLOW},
					{"gray",colors::GRAY},
					{"nil",colors::GRAY},
					{"all",colors::GRAY}
				};
				cmdss >> arg >> trg.pos.x >> trg.pos.y;
				trg.color = str2col[arg];
				if (showtrail) {
					trail.del();
					trailvbo.del();
					showtrail = false;
				}
#ifdef BRUTEFORCER_INCLUDED
				bfcurrPath = 0;
#endif
			} else if (cmd == "exit") {
				glfwSetWindowShouldClose(w, GLFW_TRUE);
			} else if (cmd == "u" || cmd == "undo") {
				cl.send(packet(packets::C_S_UNDO, nullptr, 0));
			} else if (cmd == "r" || cmd == "reset") {
				cl.send(packet(packets::C_S_RESET, nullptr, 0));
			} else if (cmd == "gu" || cmd == "giveup") {
				cl.send(packet(packets::C_S_GIVE_UP, nullptr, 0));
			} else {
				writeChat(std::string("Unknown command: \"") + cmd + "\"");
			}
		} else {
			packet p(packets::C_S_MESSAGE, tbgame.text.c_str(), tbgame.text.size());
			cl.send(p);
		}
		tbgame.text = "";
		tbgame.cursorpos = 0;
	}
}
void app::writeChat(const std::string &str) {
#ifdef BRUTEFORCER_INCLUDED
	bfappchatmut.lock();
#endif
	size_t lines = std::count(lbchat.text().begin(), lbchat.text().end(), '\n') + 1;
	if (lines >= cfg.maxChatLines) {
		lbchat.setText(lbchat.text().substr(lbchat.text().find('\n') + 1) + "\n" + str);
	} else {
		lbchat.setText(lbchat.text() + "\n" + str);
	}
#ifdef BRUTEFORCER_INCLUDED
	bfappchatmut.unlock();
#endif
}

void app::setSun() {
	float dayprogress = getDayProgress();
	float sunangle = (dayprogress + 1)*2.f*glm::pi<float>();
	sunpos = glm::vec3(cosf(sunangle), sinf(sunangle), 0.1f);
	sunstrength = sunpos.y < 0 ? 0 : sunpos.y * 2;
	if (sunstrength > 1)
		sunstrength = 1;
	skycol = glm::vec3(.2f, .7f, 1.f) * sunstrength;
	lamp = sunpos.y < 0.1f;
	lamppos = glm::vec3(0.f, 1.0f, 0.001f);
	sunpos *= SUN_DIST;
}

void connect_cb(void *a) {
	((app *)a)->connect();
}
void tbgameWriteWrapper(void *a) {
	((app *)a)->tbgameWrite();
}

void app::initGUI() {
	glgui::init("./shaders", "./textures/font.png", glw::justPrint, glw::default_shader_error_handler());

	lbtitle.pos = glm::vec2(0, 50);
	lbtitle.anch = glgui::anchor::TOPMID;
	lbtitle.align = glgui::anchor::TOPMID;
	lbtitle.color = glm::vec3(1, 1, 1);
	lbtitle.outline = true;
	lbtitle.charsize = glm::vec2(30, 60);
	lbtitle.setText("Multiplayer-game\nclient");
	gui.controls.push_back(&lbtitle);
	tbnick.pos = glm::vec2(0, 200);
	tbnick.size = glm::vec2(600, 70);
	tbnick.anch = glgui::anchor::TOPMID;
	tbnick.align = glgui::anchor::TOPMID;
	tbnick.charsize = glm::vec2(30, 60);
	tbnick.text = cfg.defaultNick;
	gui.controls.push_back(&tbnick);
	tbip.pos = glm::vec2(0, 280);
	tbip.size = glm::vec2(600, 70);
	tbip.anch = glgui::anchor::TOPMID;
	tbip.align = glgui::anchor::TOPMID;
	tbip.charsize = glm::vec2(30, 60);
	tbip.text = cfg.defaultserv.ip;
	gui.controls.push_back(&tbip);
	tbport.pos = glm::vec2(0, 360);
	tbport.size = glm::vec2(600, 70);
	tbport.anch = glgui::anchor::TOPMID;
	tbport.align = glgui::anchor::TOPMID;
	tbport.charsize = glm::vec2(30, 60);
	tbport.text = cfg.defaultserv.port;
	gui.controls.push_back(&tbport);
	btnconnect.pos = glm::vec2(0, 440);
	btnconnect.size = glm::vec2(600, 70);
	btnconnect.anch = glgui::anchor::TOPMID;
	btnconnect.align = glgui::anchor::TOPMID;
	btnconnect.textalign = glgui::anchor::CENTER;
	btnconnect.bgcolor = glm::vec3(.7f, 1.f, 1.f);
	btnconnect.charsize = glm::vec2(30, 60);
	btnconnect.setText("connect");
	btnconnect.data = this;
	btnconnect.cb = connect_cb;
	gui.controls.push_back(&btnconnect);
	gui.pos = glm::vec2(0, 0);
	gui.size = glm::vec2(ww, wh);
	gui.anch = glgui::anchor::TOPLEFT;
	gui.align = glgui::anchor::TOPLEFT;
	gui.unfocus();
	gui.init(w);
	lbchat.pos = glm::vec2(10, -10);
	lbchat.anch = glgui::anchor::BOTLEFT;
	lbchat.align = glgui::anchor::BOTLEFT;
	lbchat.color = glm::vec3(0, 0, 0);
	lbchat.outline = true;
	lbchat.charsize = glm::vec2(12, 24);
	lbchat.setText("");
	ingamegui.controls.push_back(&lbchat);
	tbgame.pos = glm::vec2(-10, -10);
	tbgame.size = glm::vec2(300, 34);
	tbgame.anch = glgui::anchor::BOTRIGHT;
	tbgame.align = glgui::anchor::BOTRIGHT;
	tbgame.charsize = glm::vec2(12, 24);
	tbgame.text = "";
	tbgame.data = this;
	tbgame.cb = tbgameWriteWrapper;
	ingamegui.controls.push_back(&tbgame);
	lbbestpath.pos = glm::vec2(0, 10);
	lbbestpath.anch = glgui::anchor::TOPMID;
	lbbestpath.align = glgui::anchor::TOPMID;
	lbbestpath.color = glm::vec3(0, 0, 0);
	lbbestpath.outline = true;
	lbbestpath.charsize = glm::vec2(12, 24);
	lbbestpath.setText("");
	ingamegui.controls.push_back(&lbbestpath);
	ingamegui.pos = glm::vec2(0, 0);
	ingamegui.size = glm::vec2(ww, wh);
	ingamegui.anch = glgui::anchor::TOPLEFT;
	ingamegui.align = glgui::anchor::TOPLEFT;
	ingamegui.unfocus();
	ingamegui.init(w);
}
void app::update() {
	double mx, my;
	glfwGetCursorPos(w, &mx, &my);
	glm::vec2 mouse(static_cast<float>(mx), static_cast<float>(my));
	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		camorient += glm::vec2(mouse.y - prevm.y, prevm.x - mouse.x) * cfg.sensitivity;
		if (camorient.x >  1.57f) camorient.x = 1.57f;
		if (camorient.x < -1.57f) camorient.x = -1.57f;
		if (camorient.y >  glm::pi<float>()) camorient.y -= glm::pi<float>() * 2;
		if (camorient.y < -glm::pi<float>()) camorient.y += glm::pi<float>() * 2;
	}
	prevm = mouse;
	if (stg == gamestage::MENU) {
		gui.update(dt);
	} else {
		ingamegui.update(dt);
	}
	if (showtraildatadirty) {
		showtraildatadirty = false;
		if (showtrail) {
			trailvbo.del();
			trail.del();
		}
		glw::initVaoVbo(trail, trailvbo, showtraildata.data(), showtraildata.size() * sizeof(float),
			sizeof(float) * 8, {glw::vap(3),glw::vap(3,sizeof(float)*3),glw::vap(2,sizeof(float)*6)});
		traillen = showtraildata.size() / 8;
		showtrail = true;
	}
#ifdef BRUTEFORCER_INCLUDED
	if (bfmadetrail) {
		bftrailmut.lock();
		if (showtrail) {
			trailvbo.del();
			trail.del();
		}
		glw::initVaoVbo(trail, trailvbo, bftrail.data(), bftrail.size() * sizeof(float),
			sizeof(float) * 8, {glw::vap(3),glw::vap(3,sizeof(float)*3),glw::vap(2,sizeof(float)*6)});
		traillen = bftrail.size() / 8;
		showtrail = true;
		bfmadetrail = false;
		bftrailmut.unlock();
	}
	if (bfautoSendPath && bfPathLen != bfcurrPath) {
		char l = bfPathLen;
		cl.send(packet(packets::C_S_FOUND_PATH, &l, 1));
		bfcurrPath = bfPathLen;
	}
#endif
}
void app::tick() {
	update();
	render();
	glw::checkError("tick end check", glw::justPrint);
}

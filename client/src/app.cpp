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

app::app(int ww, int wh, const char *title) : ww(ww), wh(wh), cl(onRecv, this), showtrail(false), camorient(1, 0) {
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
#ifdef BRUTEFORCER_INCLUDED
		bfcurrPath = 0;
#endif
		break;
	case packets::S_C_MESSAGE:{
		std::string msg(p.data(), p.size());
		writeChat(msg);
		std::cout << "[Chat]: " << msg << std::endl;
		break;
	}
	case packets::S_C_FOUND_PATH:{
		size_t path = static_cast<size_t>(static_cast<unsigned char>(p.data()[0]));
		std::string name = std::string(p.data()+1, p.size()-1);
		std::string text("{" + name + "} found path with length " + std::to_string(path));
		writeChat(text);
		std::cout << "[FOUND PATH]: " << text << std::endl;
		if (path < bestPath) {
			path = bestPath;
			lbbestpath.setText(std::to_string(path) + " by " + name);
		}
		break;
	}
	case packets::S_C_TIMEOUT:{
		writeChat("Timeout!");
		std::cout << "[TIMEOUT]: timeout!" << std::endl;
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
void app::initRendering() {
	glw::checkError("init precheck", glw::justPrint);
	initModels();
	glw::checkError("init models check", glw::justPrint);
	initShaders();
	glw::checkError("init shaders check", glw::justPrint);
	initTextures();
	glw::checkError("init textures check", glw::justPrint);
	initFramebuffers();
	glw::checkError("init framebuffers check", glw::justPrint);
	initGUI();
	glw::checkError("init gui check", glw::justPrint);
}
void app::initModels() {
	float quadverts[] = {
		-1, -1, 0, 0,
		 1, -1, 1, 0,
		 1,  1, 1, 1,
		-1,  1, 0, 1,
	};
	unsigned int quadindices[] = {
		0, 1, 2,
		0, 2, 3
	};
	glw::initVaoVboEbo(quad, quadvbo, quadebo, quadverts, sizeof(quadverts),
		quadindices, sizeof(quadindices), sizeof(float)*4, {glw::vap(2),glw::vap(2, sizeof(float)*2)});
	boardmesh.load("./models/board.obj");
	wall.load("./models/wall.obj");
	robot.load("./models/robot.obj");
	sun.load("./models/sun.obj");
}
void app::initShaders() {
	glw::compileShaderFromFile(postsh, "./shaders/post", glw::default_shader_error_handler());
	postsh.use();
	postsh.uniform1i("tex", 0);
	postsh.uniform1i("bloom", 1);
	glw::compileShaderFromFile(sh3d, "./shaders/s3", glw::default_shader_error_handler());
	sh3d.use();
	sh3d.uniform1i("tex", 0);
	sh3d.uniform1i("texspec", 1);
	sh3d.uniform1i("sundepth", 2);
	sh3d.uniform1i("lampdepth", 3);
	glw::compileShaderFromFile(lightsh, "./shaders/light", glw::default_shader_error_handler());
	glw::compileShaderFromFile(blursh, "./shaders/pass.vert", "./shaders/blur.frag", glw::default_shader_error_handler());
	blursh.use();
	blursh.uniform1i("tex", 0);
	glw::compileShaderFromFile(trgsh, "./shaders/trg", glw::default_shader_error_handler());
	glw::compileShaderFromFile(trailsh, "./shaders/trail", glw::default_shader_error_handler());
}
void app::initTextures() {
	boardtex.gen();
	boardtex[0].bind();
	boardtex[0].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	boardtex[0].fromFile("./textures/board.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	boardtex[1].bind();
	boardtex[1].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	boardtex[1].fromFile("./textures/board_spec.png", glw::justPrint, "Could not find", GL_RGBA, GL_RGBA8);
	walltex.gen();
	walltex[0].bind();
	walltex[0].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	walltex[0].fromFile("./textures/wall.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	walltex[1].bind();
	walltex[1].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	walltex[1].fromFile("./textures/wall_spec.png", glw::justPrint, "Could not find", GL_RGBA, GL_RGBA8);
	whitetex.gen();
	whitetex.bind();
	whitetex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	whitetex.fromFile("./textures/white.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	blacktex.gen();
	blacktex.bind();
	blacktex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	blacktex.fromFile("./textures/black.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
}
void app::initFramebuffers() {
	sunfbo.gen();
	sundepth.gen();
	sundepth.bind();
	sundepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	sundepth.size = glm::ivec2(cfg.shadowSize, cfg.shadowSize);
	sundepth.upload(NULL, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	sunfbo.bind();
	sunfbo.attach(sundepth, GL_DEPTH_ATTACHMENT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	lampfbo.gen();
	lampdepth.gen();
	lampdepth.bind();
	lampdepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	lampdepth.size = glm::ivec2(cfg.shadowSize, cfg.shadowSize);
	lampdepth.upload(NULL, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	lampfbo.bind();
	lampfbo.attach(lampdepth, GL_DEPTH_ATTACHMENT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	tmpfbo.gen();
	tmptex.gen();
	tmptex.bind();
	tmptex.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	tmptex.size = glm::ivec2(ww, wh);
	tmptex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	tmpfbo.bind();
	tmpfbo.attach(tmptex, GL_COLOR_ATTACHMENT0);
	tmp2fbo.gen();
	tmp2tex.gen();
	tmp2tex.bind();
	tmp2tex.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	tmp2tex.size = glm::ivec2(ww, wh);
	tmp2tex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	tmp2fbo.bind();
	tmp2fbo.attach(tmp2tex, GL_COLOR_ATTACHMENT0);
	postfbo.gen();
	posttex.gen();
	posttex.bind();
	posttex.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	posttex.size = glm::ivec2(ww, wh);
	posttex.upload(NULL, GL_RGBA, GL_RGBA16F, GL_FLOAT);
	posttexover.gen();
	posttexover.bind();
	posttexover.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	posttexover.size = glm::ivec2(ww, wh);
	posttexover.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	postdepth.gen();
	postdepth.bind();
	postdepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	postdepth.size = glm::ivec2(ww, wh);
	postdepth.upload(NULL, GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8);
	postfbo.bind();
	postfbo.attach(posttex, GL_COLOR_ATTACHMENT0);
	postfbo.attach(posttexover, GL_COLOR_ATTACHMENT1);
	postfbo.attach(postdepth, GL_DEPTH_STENCIL_ATTACHMENT);
	glDrawBuffers(2, attachments);
	postfboms.gen();
	posttexms.gen();
	posttexms.bind();
	posttexms.size = glm::ivec2(ww, wh);
	posttexms.generate(GL_RGBA16F, cfg.antialiasSamples);
	posttexoverms.gen();
	posttexoverms.bind();
	posttexoverms.size = glm::ivec2(ww, wh);
	posttexoverms.generate(GL_RGBA8, cfg.antialiasSamples);
	postdepthms.gen();
	postdepthms.bind();
	postdepthms.size = glm::ivec2(ww, wh);
	postdepthms.generate(GL_DEPTH24_STENCIL8, cfg.antialiasSamples);
	postfboms.bind();
	postfboms.attach(posttexms, GL_COLOR_ATTACHMENT0);
	postfboms.attach(posttexoverms, GL_COLOR_ATTACHMENT1);
	postfboms.attach(postdepthms, GL_DEPTH_STENCIL_ATTACHMENT);
	glDrawBuffers(2, attachments);
	postfbocolor0.gen();
	postfbocolor0.bind();
	postfbocolor0.attach(posttex, GL_COLOR_ATTACHMENT0);
	postfbocolor0.attach(postdepth, GL_DEPTH_STENCIL_ATTACHMENT);
	postfbocolor1.gen();
	postfbocolor1.bind();
	postfbocolor1.attach(posttexover, GL_COLOR_ATTACHMENT0);
	postfbomscolor0.gen();
	postfbomscolor0.bind();
	postfbomscolor0.attach(posttexms, GL_COLOR_ATTACHMENT0);
	postfbomscolor0.attach(postdepthms, GL_DEPTH_STENCIL_ATTACHMENT);
	postfbomscolor1.gen();
	postfbomscolor1.bind();
	postfbomscolor1.attach(posttexoverms, GL_COLOR_ATTACHMENT0);
	glw::fbo::screen.bind();
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

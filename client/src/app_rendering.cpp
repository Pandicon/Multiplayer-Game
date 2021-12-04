#include "app.hpp"

void app::render() {
	renderGame();
	// draw posteffects
	if (stg == gamestage::MENU && cfg.menuBlur > 0) {
		tmp2fbo.bind();
	} else {
		glw::fbo::screen.bind();
	}
	postsh.use();
	postsh.uniform1f("exposure", cfg.exposure);
	posttex.bind(GL_TEXTURE0);
	(cfg.bloomPasses > 0 ? tmp2tex : posttexover).bind(GL_TEXTURE1);
	quad.drawElements(6);
	if (stg == gamestage::MENU) {
		bool horizontal = 1;
		quad.bind();
		blursh.use();
		for (size_t i = cfg.menuBlur * 2; i; --i, horizontal = !horizontal) {
			if (i == 1)
				glw::fbo::screen.bind();
			else
				tmpfbo.bind();
			tmp2tex.bind(GL_TEXTURE0);
			blursh.uniform1i("horizontal", horizontal);
			quad.drawElements(6);
			tmpfbo.swap(tmp2fbo);
			tmptex.swap(tmp2tex);
		}
		gui.render(glm::ortho<float>(0, ww, wh, 0));
	} else {
		ingamegui.render(glm::ortho<float>(0, ww, wh, 0));
	}
}
void app::renderGame() {
	// calculate camera view
	float camx = sinf(camorient.y) * cosf(camorient.x);
	float camy = sinf(camorient.x);
	float camz = cosf(camorient.y) * cosf(camorient.x);
	glm::vec3 cam = glm::normalize(glm::vec3(camx, camy, camz)) * CAM_DIST;
	glm::mat4 viewm = glm::lookAt(cam, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 vp = proj * viewm;
	glm::mat4 vr(1.f);
	vr = glm::rotate(vr, camorient.x, glm::vec3(1, 0, 0));
	vr = glm::rotate(vr, -camorient.y, glm::vec3(0, 1, 0));
	// target model matrix
	glm::mat4 trgmodel = glm::translate(glm::mat4(1.f),
		glm::vec3(
			trg.pos.x * 0.125f - 0.9375f,
			0.005f,
			trg.pos.y * 0.125f - 0.9375f));
	// shadow maps
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glm::mat4 sproj = glm::lookAt(sunpos / SUN_DIST * 1.2f, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	sproj = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.1f, 2.5f) * sproj;
	glm::mat4 lproj = glm::lookAt(lamppos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	lproj = glm::perspective(2.f, 1.f, 0.1f, 2.5f) * lproj;
	if (cfg.shadows) {
		sunfbo.bind();
		glViewport(0, 0, cfg.shadowSize, cfg.shadowSize);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(sproj, lightsh);
		lightsh.use();
		lightsh.uniformM4f("proj", sproj * trgmodel);
		lightsh.uniformM4f("model", trgmodel);
		renderTarget();
		if (showtrail) {
			lightsh.uniformM4f("proj", sproj);
			lightsh.uniformM4f("model", glm::mat4(1.f));
			renderTrail();
		}
		if (lamp) {
			lampfbo.bind();
			glClear(GL_DEPTH_BUFFER_BIT);
			renderScene(lproj, lightsh);
			lightsh.use();
			lightsh.uniformM4f("proj", lproj * trgmodel);
			lightsh.uniformM4f("model", trgmodel);
			renderTarget();
			if (showtrail) {
				lightsh.uniformM4f("proj", lproj);
				lightsh.uniformM4f("model", glm::mat4(1.f));
				renderTrail();
			}
		}
	}
	// scene
	if (cfg.antialias) {
		glEnable(GL_MULTISAMPLE);
		postfboms.bind();
	} else {
		postfbo.bind();
	}
	glViewport(0, 0, ww, wh);
	glDrawBuffers(1, attachments + 1);
	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawBuffers(1, attachments);
	glClearColor(skycol.x, skycol.y, skycol.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawBuffers(2, attachments);
	glDisable(GL_DEPTH_TEST);
	trgsh.use();
	glm::mat4 sunmodel(1.f);
	glm::vec3 rendsunpos = sunpos / SUN_DIST * 25.f;
	sunmodel = glm::translate(sunmodel, rendsunpos);
	trgsh.uniformM4f("proj", proj * vr * sunmodel);
	trgsh.uniformM4f("model", sunmodel);
	trgsh.uniform3f("col", sunstrength * 20, sunstrength * 20, sunstrength * 20);
	sun.vao.bind();
	sun.draw();
	glEnable(GL_DEPTH_TEST);
	sh3d.use();
	sh3d.uniform3f("suncol", sunstrength, sunstrength, sunstrength);
	sh3d.uniform3f("lampcol", .8f, .8f, .6f);
	sh3d.uniform3f("sunpos", sunpos);
	sh3d.uniform3f("lamppos", lamppos);
	sh3d.uniform1i("lampon", lamp);
	sh3d.uniform3f("campos", cam);
	sh3d.uniformM4f("sunproj", sproj);
	sh3d.uniformM4f("lampproj", lproj);
	sh3d.uniform1i("shadows", cfg.shadows);
	sundepth.bind(GL_TEXTURE2);
	lampdepth.bind(GL_TEXTURE3);
	renderScene(vp, sh3d);
	trgsh.use();
	trgsh.uniformM4f("proj", vp * trgmodel);
	trgsh.uniformM4f("model", trgmodel);
	trgsh.uniform3f("col", colors::toRGB[trg.color]);
	renderTarget();
	if (showtrail) {
		trailsh.use();
		trailsh.uniformM4f("proj", vp);
		trailsh.uniformM4f("model", glm::mat4(1.f));
		renderTrail();
	}
	if (cfg.antialias) {
		postfbomscolor0.bind(GL_READ_FRAMEBUFFER);
		postfbocolor0.bind(GL_DRAW_FRAMEBUFFER);
		postfbomscolor0.blit(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST, ww, wh, ww, wh);
		postfbomscolor1.bind(GL_READ_FRAMEBUFFER);
		postfbocolor1.bind(GL_DRAW_FRAMEBUFFER);
		postfbomscolor1.blit(GL_COLOR_BUFFER_BIT, GL_NEAREST, ww, wh, ww, wh);
	}
	// blur of bloom
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	bool horizontal = 1;
	quad.bind();
	blursh.use();
	for (size_t i = 0; i < cfg.bloomPasses * 2; ++i, horizontal = !horizontal) {
		tmpfbo.bind();
		if (i == 0)
			posttexover.bind(GL_TEXTURE0);
		else
			tmp2tex.bind(GL_TEXTURE0);
		blursh.uniform1i("horizontal", horizontal);
		quad.drawElements(6);
		tmpfbo.swap(tmp2fbo);
		tmptex.swap(tmp2tex);
	}
}
void app::renderScene(const glm::mat4 &vp, glw::shader &sh) {
	sh.use();
	sh.uniformM4f("proj", vp * glm::mat4(1.f));
	sh.uniformM4f("model", glm::mat4(1.f));
	sh.uniform3f("col", 1, 1, 1);
	boardtex[0].bind(GL_TEXTURE0);
	boardtex[1].bind(GL_TEXTURE1);
	boardmesh.vao.bind();
	boardmesh.draw();

	for (size_t x = 0; x < 16; ++x) {
		for (size_t y = 0; y < 16; ++y) {
			for (size_t side = 0; side < 4; ++side) {
				if (brd.walls[x][y][side]) {
					glm::mat4 model = glm::mat4(1.f);
					model = glm::translate(model, glm::vec3(x * 0.125f - 0.9375f, 0, y * 0.125f - 0.9375f));
					model = glm::rotate(model, glm::pi<float>() * .5f * (3-side), glm::vec3(0, 1, 0));
					sh.uniformM4f("proj", vp * model);
					sh.uniformM4f("model", model);
					sh.uniform3f("col", 1, 1, 1);
					walltex[0].bind(GL_TEXTURE0);
					walltex[1].bind(GL_TEXTURE1);
					wall.vao.bind();
					wall.draw();
				}
			}
		}
	}
	for (size_t i = 0; i < 5; ++i) {
		glm::mat4 model = glm::translate(glm::mat4(1.f),
			glm::vec3(
				bots[i].pos.x * 0.125f - 0.9375f,
				0.001f,
				bots[i].pos.y * 0.125f - 0.9375f));
		sh.uniformM4f("proj", vp * model);
		sh.uniformM4f("model", model);
		sh.uniform3f("col", colors::toRGB[bots[i].color]);
		whitetex.bind(GL_TEXTURE0);
		blacktex.bind(GL_TEXTURE1);
		robot.vao.bind();
		robot.draw();
	}
}
void app::renderTarget() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	robot.vao.bind();
	robot.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void app::renderTrail() {
	glLineWidth(3);
	trail.bind();
	trail.drawArrays(traillen, GL_LINES);
	glLineWidth(1);
}

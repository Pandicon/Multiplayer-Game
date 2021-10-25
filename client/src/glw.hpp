#ifndef __GLW_HPP__
#define __GLW_HPP__

#include <array>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

/**
 *  @brief C++ wrappers for OpenGL objects
 *  @ingroup opengl
 */
namespace glw {
	/**
	 *  @brief utility for glw
	 *  @ingroup opengl
	 */
	namespace util {
		/**
		 * @brief loads file and returns contents as stringstream
		 * @param fn path to file
		 * @return std::stringstream containing file contents
		 */
		inline std::stringstream loadFile_ss(const std::string &fn) noexcept {
			std::ifstream f(fn);
			std::stringstream ss;
			ss << f.rdbuf();
			return ss;
		}
		/**
		 * @brief loads file and returns contents as string
		 * @param fn path to file
		 * @return std::string containing file contents
		 */
		inline std::string loadFile(const std::string &fn) noexcept { return loadFile_ss(fn).str(); }
	}
	// --------------------------------------------------- CONSTANTS --------------------------------------
	
	/**
	 * @brief constant used as size of error message buffer for shader infologs
	 */
	constexpr size_t INFOLOG_BUFFER_SIZE = 1024;
	
	// ------------------------------------------ ERRORS -----------------------------------

	/**
	 * @brief checks for OpenGL errors and sends them to errorHandler
	 * @param msg message for errorHandler
	 * @param file __FILE__ of check (added by macro)
	 * @param line __LINE__ of check (added by macro)
	 * @param errorHandler function/method/functor which takes error in format "{error} {msg} - from check in {file}:{line}" and processes error further (usually prints)
	 */
	template<typename F>
	bool checkError_(const std::string &msg, const char *file, int line, F errorHandler) {
		GLenum errorCode;
		bool error = false;
		while ((errorCode = glGetError()) != GL_NO_ERROR) {
			std::string error;
			switch (errorCode) {
				case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
				case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
				case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
				case 0x503:                            error = "STACK_OVERFLOW"; break;
				case 0x504:                            error = "STACK_UNDERFLOW"; break;
				case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
				case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			}
			errorHandler(error + " " + msg + " - from check in " + file + ":" + std::to_string(line));
			error = true;
		}
		return error;
	}
#define checkError(msg, h) checkError_(msg, __FILE__, __LINE__, h)

	/**
	 * @brief prints string, is meant to be used as basic error handler
	 * @param s printed string
	 */
	inline void justPrint(const std::string &s) { std::cout << s << std::endl; }

	// ------------------------------------------------------- ARRAY TYPE -----------------------------------

	typedef void (*genFunT)(GLsizei, GLuint *);
	typedef void (*delFunT)(GLsizei, const GLuint *);

	template<size_t size, typename T, genFunT genFun, delFunT delFun>
	class glarray {
	public:
		unsigned int ids[size];
		bool exists;
		inline glarray() { exists = false; }
		inline ~glarray() { if (exists) delFun(size, ids); }
		inline void gen() { genFun(size, ids); exists = true; }
		inline void del() { delFun(size, ids); exists = false; }
		inline T operator[](size_t i) const {
			T o;
			o.id = ids[i];
			o.exists = false; // So it doesn't delete the object in destructor
			return o;
		}
	};
	template<typename T, genFunT genFun, delFunT delFun>
	class dynamic_glarray {
	public:
		unsigned int *ids;
		size_t size;
		bool exists;
		inline dynamic_glarray() { exists = false; ids = nullptr; }
		inline ~dynamic_glarray() { if (exists) { delFun(size, ids); delete[] ids; } }
		inline void gen(size_t s) { size = s; ids = new unsigned int[size]; genFun(size, ids); exists = true; }
		inline void del() { delFun(size, ids); delete[] ids; ids = nullptr; size = 0; exists = false; }
		inline T operator[](size_t i) const {
			T o;
			o.id = ids[i];
			o.exists = false; // So it doesn't delete the object in destructor
			return o;
		}
	};

	// ------------------------------------------------------- BUFFERS --------------------------------------

	/**
	 * @brief wrapper for OpenGL Buffers - 
	 * buffers are used to store blocks of data in GPU memory
	 * @tparam defaulttarget default target the buffer uses
	 */
	template<GLenum defaulttarget>
	class buffer {
	public:
		/**
		 * @brief id of OpenGL buffer
		 */
		unsigned int id;
		/**
		 * @brief is buffer generated?
		 */
		bool exists;
		/**
		 * @brief creates ungenerated OpenGL buffer
		 */
		inline buffer() { id = 0; exists = false; }
		/**
		 * @brief destroys buffer if {exists}
		 */
		inline ~buffer() { if (exists) glDeleteBuffers(1, &id); }
		/**
		 * @brief generates buffer (into {id})
		 */
		inline void gen() { glGenBuffers(1, &id); exists = true; }
		/**
		 * @brief deletes buffer
		 */
		inline void del() { glDeleteBuffers(1, &id); exists = false; }
		/**
		 * @brief binds buffer to target
		 * @param target binding target (default {defaulttarget})
		 */
		inline void bind(GLenum target=defaulttarget) const { glBindBuffer(target, id); }
		/**
		 * @brief uploads data to bound buffer
		 * @param data data to upload
		 * @param size length of data in bytes
		 * @param usage usage of data (default GL_STATIC_DRAW)
		 * @param target binding target (default {defaulttarget})
		 */
		inline void upload(const void *data, size_t size, GLenum usage=GL_STATIC_DRAW, GLenum target=defaulttarget) { glBufferData(target, size, data, usage); }
		/**
		 * @brief updates part of/all data of bound buffer
		 * @param data data to upload
		 * @param size length of data in bytes
		 * @param offset offset from start of buffer (default 0)
		 * @param target binding target (default {defaulttarget})
		 */
		inline void update(const void *data, size_t size, size_t off=0, GLenum target=defaulttarget) { glBufferSubData(target, off, size, data); }
	};
	inline void call_glGenBuffers(GLsizei s, GLuint *ids) { glGenBuffers(s, ids); }
	inline void call_glDeleteBuffers(GLsizei s, const GLuint *ids) { glDeleteBuffers(s, ids); }
	/**
	 * @brief wrapper for array of OpenGL buffers
	 * @tparam size size of array
	 * @tparam trg default target of buffers
	 */
	template<size_t size, GLenum trg>
	using buffer_array = glarray<size, buffer<trg>, call_glGenBuffers, call_glDeleteBuffers>;
	/**
	 * @brief wrapper for dynamic array of OpenGL buffers
	 * @tparam trg default target of buffers
	 */
	template<GLenum trg>
	using dynamic_buffer_array = dynamic_glarray<buffer<trg>, call_glGenBuffers, call_glDeleteBuffers>;

	using vbo = buffer<GL_ARRAY_BUFFER>;
	template<size_t size>
	using vbo_array = buffer_array<size, GL_ARRAY_BUFFER>;
	using dynamic_vbo_array = dynamic_buffer_array<GL_ARRAY_BUFFER>;
	using ebo = buffer<GL_ELEMENT_ARRAY_BUFFER>;
	template<size_t size>
	using ebo_array = buffer_array<size, GL_ELEMENT_ARRAY_BUFFER>;
	using dynamic_ebo_array = dynamic_buffer_array<GL_ELEMENT_ARRAY_BUFFER>;
	/**
	 * @brief vertex attribute pointer info
	 */
	struct vap {
	public:
		/**
		 * @brief dimensions of value
		 */
		size_t size;
		/**
		 * @brief offset of value in bytes
		 */
		size_t offset;
		/**
		 * @brief should the value be remapped to range <-1, 1> or <0, 1>?
		 */
		bool normalized;
		/**
		 * @brief value data type
		 */
		GLenum type;
		/**
		 * @brief vertex attribute pointer info ctor
		 * @param size dimensions of value
		 * @param offset offset of value in bytes (default 0)
		 * @param type value data type (default GL_FLOAT)
		 * @param normalized should the value be remapped to range <-1, 1> or <0, 1>? (default false)
		 */
		vap(size_t size, size_t offset=0, GLenum type=GL_FLOAT, bool normalized=false) :
			size(size), offset(offset), normalized(normalized), type(type) { }
	};
	/**
	 * @brief wrapper for vertex array object - 
	 * vaos are used to store how is buffer data inseted into shaders
	 */
	class vao {
	public:
		/**
		 * @brief id of vao
		 */
		unsigned int id;
		/**
		 * @brief is vao generated?
		 */
		bool exists;
		/**
		 * @brief creates ungenerated OpenGL vao
		 */
		inline vao() { id = 0; exists = false; }
		/**
		 * @brief destroys vao if {exists}
		 */
		inline ~vao() { if (exists) glDeleteVertexArrays(1, &id); }
		/**
		 * @brief generates vao (into {id})
		 */
		inline void gen() { glGenVertexArrays(1, &id); exists = true; }
		/**
		 * @brief deletes vao
		 */
		inline void del() { glDeleteVertexArrays(1, &id); exists = false; }
		/**
		 * @brief binds vao
		 */
		inline void bind() const { glBindVertexArray(id); }
		/**
		 * @brief draws bound vao
		 * @param verts number of verticies to draw
		 * @param mode primitive to draw
		 * @param off first vertex offset
		 */
		inline void drawArrays(size_t verts, GLenum mode=GL_TRIANGLES, size_t off=0) const { glDrawArrays(mode, off, verts); }
		inline void drawArraysInstanced(size_t instances, size_t verts, GLenum mode=GL_TRIANGLES, size_t off=0) const { glDrawArraysInstanced(mode, off, verts, instances); }
		inline void drawElements(size_t verts, GLenum mode=GL_TRIANGLES, GLenum itype=GL_UNSIGNED_INT, size_t off=0) const { glDrawElements(mode, verts, itype, (void *)off); }
		inline void setVAP(unsigned int i, size_t stride, const vap &v) {
			glVertexAttribPointer(i, v.size, v.type, v.normalized, stride, (void *)v.offset);
			glEnableVertexAttribArray(i);
		}
		inline void setVAPs(size_t stride, const std::initializer_list<vap> &vaps) {
			int i = 0;
			for (const vap &v : vaps) {
				setVAP(i++, stride, v);
			}
		}
		inline void disableVAPs(size_t count) {
			for (int i = count; i;) {
				glDisableVertexAttribArray(--i);
			}
		}
		static vao null;
	};
	inline void call_glGenVertexArrays(GLsizei s, GLuint *ids) { glGenVertexArrays(s, ids); }
	inline void call_glDeleteVertexArrays(GLsizei s, const GLuint *ids) { glDeleteVertexArrays(s, ids); }
	template<size_t size>
	using vao_array = glarray<size, vao, call_glGenVertexArrays, call_glDeleteVertexArrays>;
	using dynamic_vao_array = dynamic_glarray<vao, call_glGenVertexArrays, call_glDeleteVertexArrays>;

	inline void initVaoVbo(vao &a, vbo &b, const void *verts, size_t vsize, size_t stride,
		const std::initializer_list<vap> &vaps, GLenum usage=GL_STATIC_DRAW) {
		a.gen();
		b.gen();
		a.bind();
		b.bind();
		b.upload(verts, vsize, usage);
		a.setVAPs(stride, vaps);
	}
	inline void initVaoVboEbo(vao &a, vbo &b, ebo &e, const void *verts, size_t vsize, const void *indices,
		size_t isize, size_t stride, const std::initializer_list<vap> &vaps,
		GLenum usage=GL_STATIC_DRAW) {
		a.gen();
		b.gen();
		e.gen();
		a.bind();
		b.bind();
		b.upload(verts, vsize, usage);
		e.bind();
		e.upload(indices, isize, usage);
		a.setVAPs(stride, vaps);
	}

	// --------------------------------------------------- SHADERS ----------------------------------------------

	inline std::string shaderTypeToStr(GLenum type) {
		switch (type) {
		case GL_VERTEX_SHADER: return "VERTEX SHADER";
		case GL_FRAGMENT_SHADER: return "FRAGMENT SHADER";
		case GL_GEOMETRY_SHADER: return "GEOMETRY SHADER";
		default: return std::to_string(type);
		}
	}
	class default_shader_error_handler {
	public:
		inline void compilation(unsigned int id, GLenum type, int compiled, char *infoLog) const {
			(void) id; (void) compiled;
			std::cout << "FAILED COMPILING " << shaderTypeToStr(type) << ": " << infoLog << std::endl;
		}
		inline void linking(unsigned int id, int compiled, char *infoLog) const {
			(void) id; (void) compiled;
			std::cout << "FAILED LINKING: " << infoLog << std::endl;
		}
	};
	/**
	 * @brief wrapper for shader - 
	 * shaders are used to process data on GPU
	 * @tparam type type of shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER or GL_GEOMETRY_SHADER)
	 */
	template<GLenum type>
	class shader_part {
	public:
		/**
		 * @brief id of shader part
		 */
		unsigned int id;
		/**
		 * @brief is shader part generated?
		 */
		bool exists;
		/**
		 * @brief creates ungenerated shader part
		 */
		inline shader_part() { exists = false; id = 0; }
		/**
		 * @brief destroys shader part if {exists}
		 */
		inline ~shader_part() { if (exists) glDeleteShader(id); }
		/**
		 * @brief generates shader part (into {id})
		 */
		inline void gen() { id = glCreateShader(type); exists = true; }
		/**
		 * @brief deletes shaderpart
		 */
		inline void del() { glDeleteShader(id); exists = false; }
		inline void setSource(const char *src) { glShaderSource(id, 1, &src, NULL); }
		template<typename F>
		inline void compile(F errorHandler) {
			glCompileShader(id);
			int compiled;
			glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
			if (!compiled) {
				char infolog[INFOLOG_BUFFER_SIZE];
				glGetShaderInfoLog(id, INFOLOG_BUFFER_SIZE, NULL, infolog);
				errorHandler.compilation(id, type, compiled, infolog);
			}
		}
	};
	using vertex_shader = shader_part<GL_VERTEX_SHADER>;
	using fragment_shader = shader_part<GL_FRAGMENT_SHADER>;
	/**
	 * @brief wrapper for shader program - 
	 * shader programs are compiled shaders which are used to process data on GPU
	 */
	class shader {
	public:
		/**
		 * @brief id of shader
		 */
		unsigned int id;
		/**
		 * @brief is shader generated?
		 */
		bool exists;
		/**
		 * @brief creates ungenerated shader
		 */
		inline shader() { id = 0; exists = false; }
		/**
		 * @brief destroys shader if {exists}
		 */
		inline ~shader() { if (exists) glDeleteProgram(id); }
		/**
		 * @brief generates shader (into {id})
		 */
		inline void gen() { id = glCreateProgram(); exists = true; }
		/**
		 * @brief deletes shader
		 */
		inline void del() { glDeleteProgram(id); exists = false; }
		inline void use() const { glUseProgram(id); }
		inline void uniform1u(const char *name, unsigned int val) { glUniform1ui(glGetUniformLocation(id, name), val); }
		inline void uniform1i(const char *name, int          val) { glUniform1i (glGetUniformLocation(id, name), val); }
		inline void uniform1f(const char *name, float        val) { glUniform1f (glGetUniformLocation(id, name), val); }
		inline void uniform2u(const char *name, unsigned int x, unsigned int y) { glUniform2ui(glGetUniformLocation(id, name), x, y); }
		inline void uniform2i(const char *name, int          x, int          y) { glUniform2i (glGetUniformLocation(id, name), x, y); }
		inline void uniform2f(const char *name, float        x, float        y) { glUniform2f (glGetUniformLocation(id, name), x, y); }
		inline void uniform3u(const char *name, unsigned int x, unsigned int y, unsigned int z) { glUniform3ui(glGetUniformLocation(id, name), x, y, z); }
		inline void uniform3i(const char *name, int          x, int          y, int          z) { glUniform3i (glGetUniformLocation(id, name), x, y, z); }
		inline void uniform3f(const char *name, float        x, float        y, float        z) { glUniform3f (glGetUniformLocation(id, name), x, y, z); }
		inline void uniform4u(const char *name, unsigned int x, unsigned int y, unsigned int z, unsigned int w) { glUniform4ui(glGetUniformLocation(id, name), x, y, z, w); }
		inline void uniform4i(const char *name, int          x, int          y, int          z, int          w) { glUniform4i (glGetUniformLocation(id, name), x, y, z, w); }
		inline void uniform4f(const char *name, float        x, float        y, float        z, float        w) { glUniform4f (glGetUniformLocation(id, name), x, y, z, w); }
		inline void uniform2u(const char *name, const glm::uvec2 &v) { uniform2u(name, v.x, v.y); }
		inline void uniform2i(const char *name, const glm::ivec2 &v) { uniform2i(name, v.x, v.y); }
		inline void uniform2f(const char *name, const glm::fvec2 &v) { uniform2f(name, v.x, v.y); }
		inline void uniform3u(const char *name, const glm::uvec3 &v) { uniform3u(name, v.x, v.y, v.z); }
		inline void uniform3i(const char *name, const glm::ivec3 &v) { uniform3i(name, v.x, v.y, v.z); }
		inline void uniform3f(const char *name, const glm::fvec3 &v) { uniform3f(name, v.x, v.y, v.z); }
		inline void uniform4u(const char *name, const glm::uvec4 &v) { uniform4u(name, v.x, v.y, v.z, v.w); }
		inline void uniform4i(const char *name, const glm::ivec4 &v) { uniform4i(name, v.x, v.y, v.z, v.w); }
		inline void uniform4f(const char *name, const glm::fvec4 &v) { uniform4f(name, v.x, v.y, v.z, v.w); }
		inline void uniformM2f(const char *name, const glm::mat2 &m) { glUniformMatrix2fv(glGetUniformLocation(id, name), 1, GL_FALSE, glm::value_ptr(m)); }
		inline void uniformM3f(const char *name, const glm::mat3 &m) { glUniformMatrix3fv(glGetUniformLocation(id, name), 1, GL_FALSE, glm::value_ptr(m)); }
		inline void uniformM4f(const char *name, const glm::mat4 &m) { glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, glm::value_ptr(m)); }
		template<GLenum type>
		inline void attach(shader_part<type> &sp) { glAttachShader(id, sp.id); }
		template<typename F>
		inline void link(F errorHandler) {
			glLinkProgram(id);
			int compiled;
			glGetProgramiv(id, GL_LINK_STATUS, &compiled);
			if (!compiled) {
				char infolog[INFOLOG_BUFFER_SIZE];
				glGetProgramInfoLog(id, INFOLOG_BUFFER_SIZE, NULL, infolog);
				errorHandler.linking(id, compiled, infolog);
			}
		}
	};
	template<typename F>
	inline void compileShader(shader &sh, const char *vsrc, const char *fsrc, F errorHandler) {
		vertex_shader vs;
		vs.gen();
		vs.setSource(vsrc);
		vs.compile(errorHandler);
		fragment_shader fs;
		fs.gen();
		fs.setSource(fsrc);
		fs.compile(errorHandler);
		sh.gen();
		sh.attach(vs);
		sh.attach(fs);
		sh.link(errorHandler);
	}
	template<typename F>
	inline void compileShaderFromFile(shader &sh, const char *vfile, const char *ffile, F errorHandler) {
		compileShader(sh, util::loadFile(vfile).c_str(), util::loadFile(ffile).c_str(), errorHandler);
	}
	template<typename F>
	inline void compileShaderFromFile(shader &sh, const char *filebase, F errorHandler) {
		compileShader(sh, util::loadFile(std::string(filebase) + ".vert").c_str(), util::loadFile(std::string(filebase) + ".frag").c_str(), errorHandler);
	}
	
	// -------------------------------------- TEXTURES ----------------------------------------------

	/**
	 * @brief wrapper for texture - 
	 * texture is used to pass big amounts of data into shaders, usualy textures
	 * @tparam dims dimensions of texture
	 * @tparam texture target
	 */
	template<size_t dims, GLenum gldims>
	class texture {
	public:
		/**
		 * @brief id of texture
		 */
		unsigned int id;
		/**
		 * @brief is texture generated?
		 */
		bool exists;
		glm::vec<dims, int> size;
		int channels;
		/**
		 * @brief creates ungenerated texture
		 */
		inline texture() { id = 0; exists = false; }
		/**
		 * @brief destroys texture if {exists}
		 */
		inline ~texture() { if (exists) glDeleteTextures(1, &id); }
		/**
		 * @brief generates texture (into {id})
		 */
		inline void gen() { glGenTextures(1, &id); exists = true; }
		/**
		 * @brief deletes texture
		 */
		inline void del() { glDeleteTextures(1, &id); exists = false; }
		inline void bind() const { glBindTexture(gldims, id); }
		inline void bind(GLenum unit) const { glActiveTexture(unit); glBindTexture(gldims, id); }
		inline void genMipmaps() { glGenerateMipmap(gldims); }
		inline void upload(const void *data, GLenum usedChannels=GL_RGBA, GLenum dataChannels=GL_RGBA8, GLenum datatype=GL_UNSIGNED_BYTE) {
			     if constexpr (dims == 1) glTexImage1D(gldims, 0, dataChannels, size.x, 0, usedChannels, datatype, data);
			else if constexpr (dims == 2) glTexImage2D(gldims, 0, dataChannels, size.x, size.y, 0, usedChannels, datatype, data);
			else if constexpr (dims == 3) glTexImage3D(gldims, 0, dataChannels, size.x, size.y, size.z, 0, usedChannels, datatype, data);
		}
		inline static void setWrapping(std::array<GLenum, dims> wraps) {
			glTexParameteri(gldims, GL_TEXTURE_WRAP_S, wraps[0]);
			if constexpr (dims > 1) {
				glTexParameteri(gldims, GL_TEXTURE_WRAP_T, wraps[1]);
				if constexpr (dims > 2)
					glTexParameteri(gldims, GL_TEXTURE_WRAP_R, wraps[2]);
			}
		}
		inline static void setBorderCol(float r, float g, float b, float a) {
			float borderColor[] = { r, g, b, a };
			glTexParameterfv(gldims, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
		inline static void setBorderCol(const glm::vec4 &col) {
			glTexParameterfv(gldims, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(col));
		}
		inline static void setFiltering(GLenum min, GLenum mag) {
			glTexParameteri(gldims, GL_TEXTURE_MIN_FILTER, min);
			glTexParameteri(gldims, GL_TEXTURE_MAG_FILTER, mag);
		}
		inline static void setWrapFilter(std::array<GLenum, dims> wraps, GLenum min, GLenum mag) {
			setWrapping(wraps);
			setFiltering(min, mag);
		}
	};

	using texture1 = texture<1, GL_TEXTURE_1D>;
	using texture2 = texture<2, GL_TEXTURE_2D>;
	using texture3 = texture<3, GL_TEXTURE_3D>;

	using tex1 = texture1;
	/**
	 * @brief wrapper for 2D texture - 
	 * texture is used to pass big amounts of data into shaders, usualy textures
	 */
	class tex2 : public texture2 {
	public:
		inline unsigned char *load(const char *file) {
			return stbi_load(file, &size.x, &size.y, &channels, 0);
		}
		inline void freedata(unsigned char *data) {
			stbi_image_free(data);
		}
		template<typename F>
		inline void fromFile(const char *file, F onError, const std::string &emsg, GLenum usedChannels=GL_RGBA, GLenum dataChannels=GL_RGBA8) {
			unsigned char *data = load(file);
			if (data) {
				upload(data, usedChannels, dataChannels, GL_UNSIGNED_BYTE);
			} else {
				onError(emsg + " " + file);
			}
			freedata(data);
		}
	};
	using tex3 = texture3;

	inline void call_glGenTextures(GLsizei s, GLuint *ids) { glGenTextures(s, ids); }
	inline void call_glDeleteTextures(GLsizei s, const GLuint *ids) { glDeleteTextures(s, ids); }
	template<size_t size, typename TexT>
	using texture_array = glarray<size, TexT, call_glGenTextures, call_glDeleteTextures>;
	template<typename TexT>
	using dynamic_texture_array = dynamic_glarray<TexT, call_glGenTextures, call_glDeleteTextures>;
	template<size_t size>
	using tex1_array = texture_array<size, tex1>;
	using tex1_dynamic_array = dynamic_texture_array<tex1>;
	template<size_t size>
	using tex2_array = texture_array<size, tex2>;
	using tex2_dynamic_array = dynamic_texture_array<tex2>;
	template<size_t size>
	using tex3_array = texture_array<size, tex3>;
	using tex3_dynamic_array = dynamic_texture_array<tex3>;

	template<typename F>
	void loadTex2(tex2 &t, const char *file, F onErr, const std::string &emsg,
		GLenum wraps=GL_CLAMP_TO_EDGE, GLenum wrapt=GL_CLAMP_TO_EDGE, GLenum min=GL_LINEAR, GLenum mag=GL_LINEAR,
		GLenum usedChannels=GL_RGBA, GLenum dataChannels=GL_RGBA8) {
		t.gen();
		t.bind();
		t.setWrapFilter({wraps, wrapt}, min, mag);
		t.fromFile(file, onErr, emsg, usedChannels, dataChannels);
	}

	// -------------------------------------------- FRAMEBUFFERS ----------------------------------

	/**
	 * @brief wrapper for renderbuffer
	 */
	class rbo {
	public:
		/**
		 * @brief id of renderbuffer
		 */
		unsigned int id;
		/**
		 * @brief is renderbuffer generated?
		 */
		bool exists;
		/**
		 * @brief creates ungenerated renderbuffer
		 */
		inline rbo() { id = 0; exists = false; }
		/**
		 * @brief destroys renderbuffer if {exists}
		 */
		inline ~rbo() { if (exists) glDeleteRenderbuffers(1, &id); }
		/**
		 * @brief generates renderbuffer (into {id})
		 */
		inline void gen() { glGenRenderbuffers(1, &id); exists = true; }
		/**
		 * @brief deletes renderbuffer
		 */
		inline void del() { glDeleteRenderbuffers(1, &id); exists = false; }
		inline void bind(GLenum target=GL_RENDERBUFFER) const { glBindRenderbuffer(target, id); }
		inline void allocate(int w, int h, GLenum format, GLenum target=GL_RENDERBUFFER) { glRenderbufferStorage(target, format, w, h); }
	};
	/**
	 * @brief wrapper for framebuffer - 
	 * framebuffers are used to capture draw calls onto texture
	 */
	class fbo {
	public:
		/**
		 * @brief id of framebuffer
		 */
		unsigned int id;
		/**
		 * @brief is framebuffer generated?
		 */
		bool exists;
		/**
		 * @brief creates ungenerated framebuffer
		 */
		inline fbo() { id = 0; exists = false; }
		/**
		 * @brief destroys renderbuffer if {exists}
		 */
		inline ~fbo() { if (exists) glDeleteFramebuffers(1, &id); }
		/**
		 * @brief generates framebuffer (into {id})
		 */
		inline void gen() { glGenFramebuffers(1, &id); exists = true; }
		/**
		 * @brief deletes framebuffer
		 */
		inline void del() { glDeleteFramebuffers(1, &id); exists = false; }
		inline void bind(GLenum target=GL_FRAMEBUFFER) const { glBindFramebuffer(target, id); }
		inline void attach(const texture1 &t, GLenum attachment=GL_COLOR_ATTACHMENT0, GLenum target=GL_FRAMEBUFFER) { glFramebufferTexture1D(target, attachment, GL_TEXTURE_1D, t.id, 0); }
		inline void attach(const texture2 &t, GLenum attachment=GL_COLOR_ATTACHMENT0, GLenum target=GL_FRAMEBUFFER) { glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, t.id, 0); }
		inline void attach(const texture3 &t, GLenum attachment=GL_COLOR_ATTACHMENT0, int zoffset=0, GLenum target=GL_FRAMEBUFFER) { glFramebufferTexture3D(target, attachment, GL_TEXTURE_3D, t.id, 0, zoffset); }
		inline void attach(const rbo &r, GLenum attachment=GL_COLOR_ATTACHMENT0, GLenum rbotarget=GL_RENDERBUFFER, GLenum target=GL_FRAMEBUFFER) { glFramebufferRenderbuffer(target, attachment, rbotarget, r.id); }
		
		inline static bool complete(GLenum target=GL_FRAMEBUFFER) { return glCheckFramebufferStatus(target) == GL_FRAMEBUFFER_COMPLETE; }
		static fbo screen;
	};

	/**
	 * @brief contains more high level interface
	 */
	namespace high {
		template<size_t w, size_t h>
		class tiledTextureAtlas {
		public:
			tex2 atlas;
			inline tiledTextureAtlas() { }
			template<typename F>
			inline void fromFile(const std::string &fn, F onError, GLenum wrapping=GL_CLAMP_TO_EDGE, GLenum minfil=GL_LINEAR, GLenum magfil=GL_NEAREST, GLenum usedChannels=GL_RGBA, GLenum dataChannels=GL_RGBA8) {
				atlas.gen();
				atlas.bind();
				atlas.setWrapFilter({wrapping, wrapping}, minfil, magfil);
				atlas.fromFile(fn.c_str(), onError, "loading texture atlas ", usedChannels, dataChannels);
			}
			inline void getuv(size_t texid, float &u1, float &v1, float &u2, float &v2) const {
				u1 = texid % h / static_cast<float>(w);
				u2 = (texid % h + 1) / static_cast<float>(w);
				v1 = texid / h / static_cast<float>(h);
				v2 = (texid / h + 1) / static_cast<float>(h);
			}
		};
		template<size_t ttaw, size_t ttah>
		inline void rendStr(const std::string &str, float x, float y, float w, float h, float space, const tiledTextureAtlas<ttaw,ttah> &font) {
			float sx = x;
			font.atlas.bind(GL_TEXTURE0);
			vao a;
			vbo b;
			initVaoVbo(a, b, NULL, sizeof(float) * 16,
				sizeof(float) * 4, {glw::vap(2),glw::vap(2,sizeof(float)*2)}, GL_DYNAMIC_DRAW);
			a.bind();
			b.bind();
			float v[16] = {
				x,     y,     0, 0,
				x + w, y,     0, 0,
				x + w, y - h, 0, 0,
				x,     y - h, 0, 0
			};
			for (auto c = str.begin(); c != str.end(); ++c) {
				if (*c == '\n') {
					x = sx;
					y += h;
					v[0] = v[12] = x;
					v[4] = v[8] = x + w;
					v[1] = v[5] = y;
					v[9] = v[13] = y - h;
					continue;
				}
				font.getuv(static_cast<size_t>(*c), v[2], v[3], v[10], v[11]);
				v[6] = v[10];
				v[14] = v[2];
				v[7] = v[3];
				v[15] = v[11];
				b.update(v, sizeof(float) * 16);
				a.drawArrays(4, GL_TRIANGLE_FAN);
				x += w + space;
				v[0] = v[12] = x;
				v[4] = v[8] = x + w;
			}
		}
	}
}

#endif

#ifdef GLW_HPP_DECLS
#undef GLW_HPP_DECLS
glw::fbo glw::fbo::screen;
glw::vao glw::vao::null;
#endif

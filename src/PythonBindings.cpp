#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <chrono>

#include "Engine.hpp"

namespace py = pybind11;

PYBIND11_MODULE(engine, m)
{
	m.doc() = "Python bindings for C++ implemented Tetris Engine";
	
	py::class_<Engine>(m, "Engine")
		.def(py::init<>())
		.def("initialize", &Engine::initialize)
		.def("helloWorld", &Engine::helloWorld)
		.def("applyMove", &Engine::applyMove)
		.def("gameLost", &Engine::gameLost)
		.def("getLinesCleared", &Engine::getLinesCleared)
		.def("getNextStep", [](Engine &self) {
			/*std::vector<int> mega = self.getNextStep();
			const int featurePerBoard = 21;
			size_t totalBoards = mega.size() / featurePerBoard;

			py::array_t<int> out(std::vector<py::ssize_t>{(py::ssize_t) totalBoards, featurePerBoard});
			std::memcpy(out.mutable_data(), mega.data(), mega.size() * sizeof(int));
			
			return out;*/
			
			//auto start = std::chrono::high_resolution_clock::now();

			auto& mega = self.getNextStep();
			
			constexpr int featurePerBoard = 23;
			size_t totalBoards = mega.size() / featurePerBoard;
			
			py::array_t<int> out(
			{
				static_cast<py::ssize_t>(totalBoards),
				static_cast<py::ssize_t>(featurePerBoard)
			}
			);
			
			std::memcpy(out.mutable_data(), mega.data(), mega.size() * sizeof(int));
			
			//auto end = std::chrono::high_resolution_clock::now();
			//std::cout << "Binding: " << std::chrono::duration<double>(end - start).count() << "s\n";
			
			return out;
		});
}
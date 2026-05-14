CC = g++
FLAGS = -std=c++20 -Wall -Werror -O3

OBJDIR = objects

CORE_SRCS = Piece.cpp Board.cpp Game.cpp Spawner.cpp
CORE_OBJS = $(addprefix $(OBJDIR)/, $(CORE_SRCS:.cpp=.o))

PY_INC = -I/mingw64/include -I/mingw64/include/python3.14
PY_LINK = -L/mingw64/lib -lpython3.14

game: $(CORE_OBJS) $(OBJDIR)/HumanEngine.o
	$(CC) $(FLAGS) $^ -o Game.exe

engine: $(CORE_OBJS) $(OBJDIR)/Engine.o $(OBJDIR)/PythonBindings.o
	$(CC) $(FLAGS) -shared $^ $(PY_LINK) -o engine.pyd

$(OBJDIR)/%.o: src/%.cpp include/Constants.hpp
	@mkdir -p $(OBJDIR)
	$(CC) $(FLAGS) -Iinclude -c $< -o $@

$(OBJDIR)/Engine.o: src/Engine.cpp include/Engine.hpp include/Constants.hpp
	@mkdir -p $(OBJDIR)
	$(CC) $(FLAGS) -Iinclude -c $< -o $@

$(OBJDIR)/PythonBindings.o: src/PythonBindings.cpp
	@mkdir -p $(OBJDIR)
	$(CC) $(FLAGS) -Iinclude $(PY_INC) -c $< -o $@

clean:
	rm -rf $(OBJDIR) Game.exe engine.pyd

run:
	./Game.exe
	
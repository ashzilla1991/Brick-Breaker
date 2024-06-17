#include <GLFW\glfw3.h>
#include <glut.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;
bool launchBall = false;
bool gameStarted = false;

enum BRICKTYPE { DESTRUCTABLE };
enum ONOFF { ON, OFF };

enum StateofGame { PLAYING, GAME_OVER };
StateofGame gameState = PLAYING;

//padd class
class Paddle
{
public:
	//paddle configuration
	float x, y;
	float width, height;
	float red, green, blue;

	Paddle(float xx, float yy, float w, float h, float r, float g, float b) : x(xx), y(yy), width(w), height(h), red(r), green(g), blue(b) {}

	//create paddle
	void CreatePaddle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		glVertex2f(x - width / 2, y - height / 2);
		glVertex2f(x + width / 2, y - height / 2);
		glVertex2f(x + width / 2, y + height / 2);
		glVertex2f(x - width / 2, y + height / 2);
		glEnd();
	}
};

//process keyboar input
void processInput(GLFWwindow* window, Paddle& paddle);

//brick class
class Brick {
public:
	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
	};

	//creating bricks
	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};

//ball class
class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.015; //speed of ball
	int direction; //1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	//checking collision of ball & bricks
	void CheckCollision(Brick* brk)
	{
		if ((x + radius > brk->x - brk->width / 2 && x - radius < brk->x + brk->width / 2) &&
			(y + radius > brk->y - brk->width / 2 && y - radius < brk->y + brk->width / 2))
		{
			if (brk->brick_type == DESTRUCTABLE)
			{
				brk->onoff = OFF;
				direction = GetRandomDirection(); //change the ball direction after brick collision
			}
		}
	}

	//generates random number for direction
	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	//updates ball position
	void MoveOneStep()
	{
		if (direction == 1)
		{
			y += speed;
		}
		else if (direction == 2)
		{
			x += speed;
		}
		else if (direction == 3)
		{
			y -= speed;
		}
		else if (direction == 4)
		{
			x -= speed;
		}
		else if (direction == 5)
		{
			y += speed;
			x += speed;
		}
		else if (direction == 6)
		{
			y += speed;
			x -= speed;
		}
		else if (direction == 7)
		{
			y -= speed;
			x += speed;
		}
		else if (direction == 8)
		{
			y -= speed;
			x -= speed;
		}
	}

	//ball rendering
	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++)
		{
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};

vector<Circle> world; //storing circles
vector<Brick> bricks; //storing bricks

//wall collision
void WallCollision(Circle& ball)
{
	//checking if ball hits side of program window
	if (ball.x + ball.radius > 1.0f || ball.x - ball.radius < -1.0f)
	{
		ball.direction = ball.GetRandomDirection();
		if (ball.x + ball.radius > 1.0f)
		{
			ball.x = 1.0f - ball.radius;
		}
		else
		{
			ball.x = -1.0f + ball.radius;
		}
	}

	//checking if ball hits top or bottom of program window
	if (ball.y + ball.radius > 1.0f || ball.y - ball.radius < -1.0f)
	{
		ball.direction = ball.GetRandomDirection();
		if (ball.y + ball.radius > 1.0f)
		{
			ball.y = 1.0f - ball.radius;
		}
		else
		{
			ball.y = -1.0f + ball.radius;
		}
	}
}

//paddle collision
void PaddleCollision(Circle& ball, Paddle& paddle)
{
	if (ball.x + ball.radius > paddle.x - paddle.width / 2 &&
		ball.x - ball.radius < paddle.x + paddle.width / 2 &&
		ball.y - ball.radius < paddle.y + paddle.height / 2 &&
		ball.y + ball.radius > paddle.y - paddle.height / 2)
	{

		float hitPos = (ball.x - paddle.x) / (paddle.width / 2);
		if (hitPos < -0.5f)
		{
			ball.direction = 6; //northwest
		}
		else if (hitPos > 0.5f)
		{
			ball.direction = 5; //northeast
		}
		else
		{
			ball.direction = 1; //north
		}
	}
}

//condition for winning game
bool WinGame() {
	for (const auto& brick : bricks)
	{
		if (brick.onoff == ON)
		{
			return false;
		}
	}
	return true;
}

//condition for losing game
bool LoseGame(const Circle& ball)
{
	return ball.y - ball.radius < -1.0f;
}

void GameOverScreen(bool win) {
	glColor3f(1.0f, 1.0f, 1.0f); //text color

	std::string gameOverText = win ? "You Win!" : "Game Over";
	glRasterPos2f(-0.2f, 0.1f); //text position
	for (const char& c : gameOverText) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); //font
	}

	std::string restartGame = "Press Q to Quit or R to Restart";
	glRasterPos2f(-0.55f, -0.1f); //text position
	for (const char& c : restartGame) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); //font
	}
}

//bricks at the top of the window
float brickWidth = 0.2f;
float brickHeight = 0.1f;
float brickSize = 0.2f;
float startX = -1.0f + brickSize / 2;
float startY = 0.7f;
float space = 0.01f;
float endY = startY + 2 * (brickSize + space);

//reset game condtions
void gameRestart(Paddle& paddle) {
	gameState = PLAYING; //sets game to playing
	gameStarted = false; //game hasn't started
	launchBall = false; //ball hasn't launched

	paddle.x = 0.0f; //reset paddle position

	world[0].x = 0.0f; //resetting ball x position
	world[0].y = paddle.y + paddle.height + world[0].radius; //resetting ball y postion
	world[0].direction = 0; //reset ball direction

	bricks.clear(); //resets bricks
	for (float y = startY; y < endY; y += brickSize + space) {
		for (float x = startX; x < 1.0f; x += brickSize + space) {
			bricks.push_back(Brick(DESTRUCTABLE, x, y, brickSize, 0.9, 0.9, 0.9));
		}
	}
}

int main(void)
{
	srand(time(NULL));

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "CS-499: Ashley English", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	Paddle paddle(0.0f, -0.7f, 0.3f, 0.05f, 0.5f, 0.5f, 0.5f);


	for (float y = startY; y < endY; y += brickSize + space)
	{
		for (float x = startX; x < 1.0f; x += brickSize + space)
		{
			for (float x = startX; x < 1.0f; x += brickSize + space)
			{
				BRICKTYPE type = DESTRUCTABLE;
				bricks.push_back(Brick(type, x, y, brickSize, 0.9, 0.9, 0.9));
			}
		}
	}

	//size of ball
	float ballRadius = 0.05f;

	//creating ball
	float ballColorR = 0.0f;
	float ballColorG = 0.0f;
	float ballColorB = 1.0f;
	Circle ball(0.0f, paddle.y + paddle.height + ballRadius, ballRadius, 0, ballRadius, ballColorR, ballColorG, ballColorB);

	world.push_back(ball);

	while (!glfwWindowShouldClose(window)) {
		// Setup view
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window, paddle);

		if (gameState == PLAYING) {
			paddle.CreatePaddle();

			if (launchBall && gameStarted) {
				world[0].speed = 0.015f;
				world[0].direction = world[0].GetRandomDirection();
				launchBall = false;
			}

			if (!gameStarted) {
				//shows press space to start
				glColor3f(1.0f, 1.0f, 1.0f); //text color
				glRasterPos2f(-0.4f, 0.0f); //text position
				for (const char& c : "Press Space to Start") { 
					glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c); //font
				}
			}

			//creating bricks
			for (auto& brick : bricks) {
				brick.drawBrick();
			}

			//ball movement and collision
			for (auto& ball : world) {
				for (auto it = bricks.begin(); it != bricks.end();) {
					ball.CheckCollision(&(*it));
					if (it->brick_type == DESTRUCTABLE && it->onoff == OFF) {
						it = bricks.erase(it);
					}
					else {
						++it;
					}
				}

				WallCollision(ball);
				PaddleCollision(ball, paddle);
				ball.MoveOneStep();
				ball.DrawCircle();
			}

			if (WinGame()) {
				gameState = GAME_OVER;
			}
			else if (LoseGame(world[0])) {
				gameState = GAME_OVER;
			}
		}
		else if (gameState == GAME_OVER) {
			GameOverScreen(WinGame());
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window, Paddle& paddle) {
	static bool spaceBarPress = false;
	static bool rKeyPress = false;
	static bool qKeyPress = false;

	//press escape to close window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	//press space to launch ball & remove game message
	if (gameState == PLAYING) {
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (!spaceBarPress) {
				launchBall = true;
				gameStarted = true;
			}
			spaceBarPress = true;
		}
		else {
			spaceBarPress = false;
		}

		//move paddle left
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			if (paddle.x - paddle.width / 2 > -1.0f) {
				paddle.x -= 0.05f;
			}
		}

		//move paddle right
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			if (paddle.x + paddle.width / 2 < 1.0f) {
				paddle.x += 0.05f;
			}
		}
	}

	//reset game with r & quit game with q
	else if (gameState == GAME_OVER) {
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { 
			if (!rKeyPress) {
				gameRestart(paddle);
			}
			rKeyPress = true;
		}
		else {
			rKeyPress = false;
		}

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			if (!qKeyPress) {
				glfwSetWindowShouldClose(window, true);
			}
			qKeyPress = true;
		}
		else {
			qKeyPress = false;
		}
	}
}

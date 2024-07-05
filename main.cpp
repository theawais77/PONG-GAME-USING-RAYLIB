#include <iostream>
#include <raylib.h>
using namespace std;

int player1_score = 0;
int player2_score = 0;
const int winning_score = 5;

Color color1 = { 251, 12, 103, 255 };
Color color2 = { 100, 255, 40, 255 };
Color color3 = { 173, 216, 230, 255 };
Color color4 = { 255, 202, 49, 255 };

Color color_left_bg = Color{ 4, 30, 66, 255 };  
Color color_right_bg = Color{ 93, 24, 65, 255 }; 
Color color_center_circle = Color{ 24, 36, 69, 255 };  
Color color_center_dot = Color{ 255, 0, 0, 255 };  

const int screen_width = 1080;
const int screen_height = 600;

int half_width = screen_width / 2;
int half_height = screen_height / 2;

float ball_radius = 15;
int speed_x = 7;
int speed_y = 7;

float paddle_width = 25;
float paddle_height = 120;
int paddle_speed = 8;

class Message
{
public:
    void DrawVictoryMessage(const char* message)
    {
        const int screenWidth = GetScreenWidth();
        const int screenHeight = GetScreenHeight();

        Rectangle gradientRec = { screenWidth / 2 - 200, screenHeight / 2 - 50, 400, 100 };
        DrawRectangleGradientEx(gradientRec, color1, color2, color3, color4);

        DrawText(message, screenWidth / 2 - MeasureText(message, 40) / 2, screenHeight / 2 - 20, 40, WHITE);
    }
};

class Ball
{
private:
    float x, y;
    int speed_x, speed_y;
    const float radius;

public:
    Ball(float init_x, float init_y, float init_radius, int speedX, int speedY)
        : x(init_x), y(init_y), radius(init_radius), speed_x(speedX), speed_y(speedY)
    {
    }

    float getRadius() const
    {
        return radius;
    }
    float getX() const
    {
        return x;
    }
    float getY() const
    {
        return y;
    }

    void Draw() const
    {
        DrawCircle(x, y, radius, color4);
    }
    void Update()
    {
        x += speed_x;
        y += speed_y;

        if (y + radius >= GetScreenHeight() || y - radius <= 0)
        {
            speed_y *= -1;
        }
    }
    void Bounce()
    {
        speed_x *= -1;
    }
    void Reposition(float new_x, float new_y)
    {
        x = new_x;
        y = new_y;
    }
    void Reset(float init_x, float init_y)
    {
        x = init_x;
        y = init_y;
    }
    void ResetBall()
    {
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;

        int speed_choice[2]{ -1, 1 };
        speed_x = 7 * speed_choice[GetRandomValue(0, 1)];
        speed_y = 7 * speed_choice[GetRandomValue(0, 1)];
    }
};
class Paddle
{ 
private:
    float x, y;
    const float width, height;
    const int speed;

public:
    Paddle(float init_x, float init_y, float init_width, float init_height, int init_speed)
        : x(init_x), y(init_y), width(init_width), height(init_height), speed(init_speed)
    {
    }

    void Draw() const
    {
        DrawRectangleRounded(Rectangle{ x, y, width, height }, 0.5, 0, WHITE);
    }

    void Update(bool isLeft)
    {
        if (isLeft)
        {
            if (IsKeyDown(KEY_W))
            {
                y -= speed;
            }
            if (IsKeyDown(KEY_S))
            {
                y += speed;
            }
        }
        else
        {
            if (IsKeyDown(KEY_UP))
            {
                y -= speed;
            }
            if (IsKeyDown(KEY_DOWN))
            {
                y += speed;
            }
        }

        if (y <= 0)
        {
            y = 0;
        }
        if (y + height >= GetScreenHeight())
        {
            y = GetScreenHeight() - height;
        }
    }

    Rectangle getRectangle() const
    {
        return Rectangle{ x, y, width, height };
    }

    void Reset(float init_x, float init_y)
    {
        x = init_x;
        y = init_y;
    }
};

class Button {
public:
    Texture2D texture;
    Vector2 position;
    Vector2 size;
    const char* label;

    Button(const char* imagePath, Vector2 pos, const char* text);

    void Draw();

    bool IsClicked() const {
        Vector2 mousePoint = GetMousePosition();
        return CheckCollisionPointRec(mousePoint, { position.x, position.y, size.x, size.y }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    }

    ~Button();
};

Button::Button(const char* imagePath, Vector2 pos, const char* text) {
    Image image = LoadImage(imagePath);
    if (!IsImageReady(image)) {
        std::cerr << "Failed to load image. Ensure the file path is correct: " << imagePath << std::endl;
        exit(-1);
    }
    ImageResize(&image, 300, 70);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);
    position = pos;
    size = { 300, 70 };
    label = text;
}

void Button::Draw() {
    Vector2 mousePoint = GetMousePosition();
    Color tint = WHITE;
    Color textColor = WHITE;

    if (CheckCollisionPointRec(mousePoint, { position.x, position.y, size.x, size.y })) {
        tint = GRAY;
        textColor = DARKGRAY;
    }

    DrawTexture(texture, position.x, position.y, tint);
    DrawText(label, position.x + (size.x - MeasureText(label, 20)) / 2, position.y + (size.y - 20) / 2, 20, textColor);
}

Button::~Button() {
    UnloadTexture(texture);
}

void ResetGame(Ball& ball, Paddle& left_paddle, Paddle& right_paddle)
{
    player1_score = 0;
    player2_score = 0;
    ball.ResetBall();
    left_paddle.Reset(15, half_height - paddle_height / 2);
    right_paddle.Reset(screen_width - 40, half_height - paddle_height / 2);
}


void playGame()
{
    Ball ball(half_width, half_height, ball_radius, speed_x, speed_y);
    Paddle left_paddle(15, half_height - paddle_height / 2, paddle_width, paddle_height, paddle_speed);
    Paddle right_paddle(screen_width - 40, half_height - paddle_height / 2, paddle_width, paddle_height, paddle_speed);

    Message msg;


    bool ballMissed = false;
    bool gameOver = false;
    bool showSettings = false; // Flag to control whether settings screen is shown
    bool exitGame = false; // Flag to control the game exit
    double resetTime = 0;

    ResetGame(ball, left_paddle, right_paddle); // Reset the game state
    SetTargetFPS(90); // FrameRates

    // Game loop
    while (!WindowShouldClose() && !showSettings && !exitGame)
    {
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE)) {
            exitGame = true;
            break;
        }

        if (!gameOver)
        {
            if (ballMissed)
            {
                if (GetTime() - resetTime >= 2.0)
                {
                    ballMissed = false;
                    ball.ResetBall();
                }
            }
            else
            {
                ball.Update();
            }

            left_paddle.Update(true);
            right_paddle.Update(false);

            // Check collision with left paddle
            if (CheckCollisionCircleRec(Vector2{ ball.getX(), ball.getY() }, ball.getRadius(), left_paddle.getRectangle()))
            {
                ball.Bounce();
                ball.Reposition(left_paddle.getRectangle().x + left_paddle.getRectangle().width + ball.getRadius(), ball.getY());
            }

            // Check collision with right paddle
            if (CheckCollisionCircleRec(Vector2{ ball.getX(), ball.getY() }, ball.getRadius(), right_paddle.getRectangle()))
            {
                ball.Bounce();
                ball.Reposition(right_paddle.getRectangle().x - ball.getRadius(), ball.getY());
            }

            // Check if the ball is missed by the paddles
            if (ball.getX() - ball.getRadius() < 0)
            {
                ballMissed = true;
                resetTime = GetTime();
                // Increment player 2 score
                player2_score++;
                ball.Reset(half_width, half_height);
            }
            else if (ball.getX() + ball.getRadius() > screen_width)
            {
                ballMissed = true;
                resetTime = GetTime();
                // Increment player 1 score
                player1_score++;
                ball.Reset(half_width, half_height);
            }

            // Check for win condition
            if (player1_score >= winning_score || player2_score >= winning_score)
            {
                gameOver = true;
            }
        }

        // Drawing during the game
        ClearBackground(color3);
        if (!gameOver)
        {
            ball.Draw();
            left_paddle.Draw();
            right_paddle.Draw();
            DrawLine(half_width, 0, half_width, screen_height, WHITE);
            DrawText(TextFormat("%i", player1_score), screen_width / 4 - 20, 20, 80, WHITE);
            DrawText(TextFormat("%i", player2_score), 3 * screen_width / 4 - 20, 20, 80, WHITE);
        }
        else
        {
            if (player1_score >= winning_score)
            {
                msg.DrawVictoryMessage("Player 1 Wins!");
            }
            else if (player2_score >= winning_score)
            {
                msg.DrawVictoryMessage("Player 2 Wins!");
            }
        }

        EndDrawing();
    }


}

int main()
{
    InitWindow(screen_width, screen_height, "Pong Game");

    Ball ball(half_width, half_height, ball_radius, speed_x, speed_y);
    Paddle left_paddle(15, half_height - paddle_height / 2, paddle_width, paddle_height, paddle_speed);
    Paddle right_paddle(screen_width - 40, half_height - paddle_height / 2, paddle_width, paddle_height, paddle_speed);

    Button startButton("resources/images/bar1.png", { screen_width / 2 - 150, 150 }, "Start");
    Button settingsButton("resources/images/bar2.png", { screen_width / 2 - 150, 250 }, "Settings");
    Button exitButton("resources/images/bar5.png", { screen_width / 2 - 150, 350 }, "Exit");

    Button userr("resources/images/bar4.png", { screen_width / 2 - 150, 200 }, "User");
    Button cpuB("resources/images/bar5.png", { screen_width / 2 - 150, 300 }, "CPU");


    bool ballMissed = false;
    bool gameOver = false;
    bool showSettings = false;
    bool exitGame = false;
    double resetTime = 0;

    while (!WindowShouldClose() && !exitGame)
    {
        BeginDrawing();
        ClearBackground(color3);

        startButton.Draw();
        settingsButton.Draw();
        exitButton.Draw();

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE)) {
            exitGame = true;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (CheckCollisionPointRec(mouse, { screen_width / 2 - 100, 100, 300, 70 })) {
                
                playGame();
            }
            else if (settingsButton.IsClicked())
            {
                showSettings = true;

                while (showSettings && !WindowShouldClose() && !exitGame) {
                    BeginDrawing();
                    ClearBackground(color3);

                    userr.Draw();
                    cpuB.Draw();

                    if (userr.IsClicked())
                    {

                    }
                    if (cpuB.IsClicked())
                    {
                        while (!WindowShouldClose())
                        {
                            BeginDrawing();
                            ClearBackground(color3);




                            EndDrawing();
                        }

                    }
                    // Back to main menu condition
                    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ESCAPE)) {
                        showSettings = false;
                        exitGame = true;
                    }

                    EndDrawing();
                }
            }
            else if (exitButton.IsClicked()) {
                exitGame = true;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
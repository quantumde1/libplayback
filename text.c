#include <raylib.h>
#include <stdlib.h>

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

int testPlayback(char* argv, int gamepadInt)
{
    // Initialization
    //---------------------------------------------------------
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    AudioStream stream = { 0 };
    
    plm_t *plm = plm_create_with_filename(argv);
    if (!plm) return 1;
    
    double framerate = plm_get_framerate(plm);
    int samplerate = plm_get_samplerate(plm);

    TraceLog(LOG_INFO, "Framerate: %f, samplerate: %d", (float)framerate, samplerate);
       
    if (plm_get_num_audio_streams(plm) > 0) 
    {
        // Initialize audio stream
        //stream = InitAudioStream(44100, 32, 2); // 44100 Hz, 16-bit, stereo
        //PlayAudioStream(stream); // Start processing stream buffer (no data loaded currently)

        // Adjust the audio lead time according to the audio_spec buffer size
        //plm_set_audio_lead_time(plm, (double)PLM_AUDIO_SAMPLES_PER_FRAME / (double)samplerate);
    }
    
    plm_set_loop(plm, TRUE);
    plm_set_audio_enabled(plm, 0);
    
    int width = plm_get_width(plm);
    int height = plm_get_height(plm);
    
    plm_frame_t *frame = NULL;
    plm_samples_t *sample = NULL;
    
    Image imFrame = { 0 };
    imFrame.width = width;
    imFrame.height = height;
    imFrame.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    imFrame.mipmaps = 1;
    imFrame.data = (unsigned char *)malloc(width * height * 3);

    Texture texture = LoadTextureFromImage(imFrame);
    
    bool pause = false;
    double baseTime = GetTime(); // Time since InitWindow()

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //-----------------------------------------------------
        if (IsKeyPressed(KEY_ENTER) || IsGamepadButtonPressed(gamepadInt, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            return 1488;
        }
        if (!pause)
        {
            // Video should run at 'framerate' fps => One new frame every 1/framerate
            double time = (GetTime() - baseTime);
            
            if (time >= (1.0 / framerate))
            {
                baseTime = GetTime();
                
                // Decode video frame
                frame = plm_decode_video(plm); // Get frame as 3 planes: Y, Cr, Cb
                if (frame) {
                    // Calculate the stride
                    int stride = width * 3; // 3 bytes per pixel for RGB
                    plm_frame_to_rgb(frame, imFrame.data, stride); // Convert (Y, Cr, Cb) to RGB on the CPU (slow)
                    UpdateTexture(texture, imFrame.data); // Update texture
                }

                // Refill audio stream if required
                /*while (IsAudioStreamProcessed(stream)) {
                    // Decode audio sample
                    sample = plm_decode_audio(plm);
                    if (sample) {
                        // Copy finished frame to audio stream
                        UpdateAudioStream(stream, sample->interleaved, sample->count * sizeof(int16_t) * 2); // For stereo
                    }
                }*/
            }
            
        }
        //-----------------------------------------------------

        // Draw
        //-----------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE); // Clear the background

        // Define the source rectangle (the whole texture)
        Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };

        // Define the destination rectangle (the whole screen)
        Rectangle dest = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };

        // Draw the texture stretched to fill the screen
        DrawTexturePro(texture, source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // Draw skip instructions
        int posY = GetScreenHeight() - 20 - 40;
        if (IsGamepadAvailable(gamepadInt)) {
            int buttonSize = 30;
            int circleCenterX = 40 + buttonSize / 2;
            int circleCenterY = posY + buttonSize / 2;
            int textYOffset = 7; // Adjust this offset based on your font and text size
            DrawCircle(circleCenterX, circleCenterY, buttonSize / 2, GREEN);
            DrawText("A", circleCenterX - 5, circleCenterY - textYOffset, 20, BLACK);
            DrawText(" to skip video", 40 + buttonSize + 5, posY, 20, WHITE);
        } else {
            DrawText("Press Enter to Skip", 40, posY, 20, WHITE);
        }

        EndDrawing();
        //-----------------------------------------------------
    }

    // De-Initialization
    //---------------------------------------------------------
    UnloadImage(imFrame);
    UnloadTexture(texture);
    
    if (plm_get_num_audio_streams(plm) > 0) {
        UnloadAudioStream(stream); // Unload audio stream
    }
    
    plm_destroy(plm);
    
    CloseWindow(); // Close window and OpenGL context
    //----------------------------------------------------------

    return 1488;
}

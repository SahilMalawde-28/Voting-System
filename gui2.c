#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "blockchain.h"
#include "avl.h"

// Screen dimensions
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;

// Colors
const SDL_Color WHITE = {255, 255, 255, 255};
const SDL_Color BLACK = {0, 0, 0, 255};
const SDL_Color BLUE = {50, 100, 200, 255};
const SDL_Color LIGHT_BLUE = {100, 150, 250, 255};
const SDL_Color RED = {255, 0, 0, 255};
const SDL_Color GREEN = {0, 255, 0, 255};
const SDL_Color GRAY = {200, 200, 200, 255};
 static char candidateID[32] = {0};

// GUI State
typedef struct {
    blockchain *bc;
    AVLTree *voterTree;
    int integrityFailed;
    char errorMessage[256];
    char inputBuffer[50];
    int currentScreen;
} GUIState;

// Function prototypes
void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
                SDL_Color color, int x, int y, SDL_Rect* destRect);
void drawButton(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
                SDL_Rect* buttonRect, SDL_Color buttonColor);
void handleMainMenuEvents(SDL_Event* e, GUIState* guiState);
void renderMainMenu(SDL_Renderer* renderer, TTF_Font* font, GUIState* guiState);

// Render text function
void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
                SDL_Color color, int x, int y, SDL_Rect* destRect) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_QueryTexture(texture, NULL, NULL, &destRect->w, &destRect->h);
    destRect->x = x;
    destRect->y = y;
    
    SDL_RenderCopy(renderer, texture, NULL, destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


// Draw button function
void drawButton(SDL_Renderer* renderer, TTF_Font* font, const char* text, 
                SDL_Rect* buttonRect, SDL_Color buttonColor) {
    // Draw button background
    SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, 255);
    SDL_RenderFillRect(renderer, buttonRect);

    // Draw button border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, buttonRect);

    // Render button text
    SDL_Color textColor = WHITE;
    SDL_Rect textRect;
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, textColor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = buttonRect->x + (buttonRect->w - textRect.w) / 2;
    textRect.y = buttonRect->y + (buttonRect->h - textRect.h) / 2;
    
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Render Main Menu

void renderMainMenu(SDL_Renderer* renderer, TTF_Font* font, GUIState* guiState) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    // Title
    SDL_Rect titleRect;
    renderText(renderer, font, "Blockchain Voting System", BLUE, 
               SCREEN_WIDTH/2 - 200, 50, &titleRect);
    // Define button positions and sizes
    SDL_Rect buttons[] = {
        {350, 200, 300, 50},  // Register Voter
        {350, 270, 300, 50},  // Cast Vote
        {350, 340, 300, 50},  // Display Candidate
        {350, 410, 300, 50}   // Exit
    };
    const char* buttonLabels[] = {
        "Register Voter", 
        "Cast Vote", 
        "display candidate", 
        "Exit"
    };
    // Render buttons
    for (int i = 0; i < 4; i++) {
        drawButton(renderer, font, buttonLabels[i], &buttons[i], 
                   i == 3 ? RED : LIGHT_BLUE);
    }
    // Render error message if any
    if (strlen(guiState->errorMessage) > 0) {
        SDL_Rect errorRect;
        renderText(renderer, font, guiState->errorMessage, RED, 
                   250, SCREEN_HEIGHT - 100, &errorRect);
    }
    SDL_RenderPresent(renderer);
}
// Render Register Voter Screen
void renderRegisterVoter(SDL_Renderer* renderer, TTF_Font* font, GUIState* guiState) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Title
    SDL_Rect titleRect;
    renderText(renderer, font, "Register Voter", BLUE, 
               SCREEN_WIDTH/2 - 100, 50, &titleRect);

    // Input box
    SDL_Rect inputBoxRect = {350, 250, 300, 50};
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderFillRect(renderer, &inputBoxRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &inputBoxRect);

    // Render input text
    SDL_Rect inputTextRect;
    renderText(renderer, font, guiState->inputBuffer, BLACK, 
               360, 260, &inputTextRect);

    // Buttons
    SDL_Rect registerButton = {350, 350, 300, 50};
    SDL_Rect backButton = {350, 420, 300, 50};
    
    drawButton(renderer, font, "Register", &registerButton, GREEN);
    drawButton(renderer, font, "Back", &backButton, RED);

    // Render error message if any
    if (strlen(guiState->errorMessage) > 0) {
        SDL_Rect errorRect;
        renderText(renderer, font, guiState->errorMessage, RED, 
                   250, SCREEN_HEIGHT - 100, &errorRect);
    }

    SDL_RenderPresent(renderer);
}

// Event handling for Register Voter
void handleRegisterVoterEvents(SDL_Event* e, GUIState* guiState, SDL_Renderer* renderer, TTF_Font* font) {
    char voterID[8];
    
    switch (e->type) {
        case SDL_MOUSEBUTTONDOWN:
            int x = e->button.x, y = e->button.y;
            
            // Back button
            if (x >= 350 && x <= 650 && y >= 420 && y <= 470) {
                guiState->currentScreen = 0;  // Back to main menu
                guiState->errorMessage[0] = '\0';
                guiState->inputBuffer[0] = '\0';
                return;
            }
            
            // Register button
            if (x >= 350 && x <= 650 && y >= 350 && y <= 400) {
                strncpy(voterID, guiState->inputBuffer, sizeof(voterID) - 1);
                voterID[sizeof(voterID) - 1] = '\0';
                
                // Check voter registration
                if (strlen(voterID) == 0) {
                    strcpy(guiState->errorMessage, "Please enter a Voter ID");
                } /*else if isVoterRegistered(guiState->voterTree->root, voterID)) {
                    strcpy(guiState->errorMessage, "Voter ID already registered");
                } */else {
                    insertVoter(guiState->voterTree, voterID);
                    strcpy(guiState->errorMessage, "Voter registered successfully");
                    saveTreeToBinaryFile(guiState->voterTree, "voter_data.bin");
                    guiState->inputBuffer[0] = '\0';
                }
            }
            break;
        
        case SDL_TEXTINPUT:
    // If inputBuffer is for Voter ID
    if (strlen(guiState->inputBuffer) < sizeof(guiState->inputBuffer) - 1) {
        strcat(guiState->inputBuffer, e->text.text);
    } 
    // If Voter ID input is full, switch to Candidate ID
    else if (strlen(candidateID) < sizeof(candidateID) - 1) {
        strcat(candidateID, e->text.text);
    }
    break;

case SDL_KEYDOWN:
    if (e->key.keysym.sym == SDLK_BACKSPACE) {
        // First try to delete from Candidate ID
        if (strlen(candidateID) > 0) {
            candidateID[strlen(candidateID) - 1] = '\0';
        } 
        // If Candidate ID is empty, delete from Voter ID
        else if (strlen(guiState->inputBuffer) > 0) {
            guiState->inputBuffer[strlen(guiState->inputBuffer) - 1] = '\0';
        }
    }
    break;
}
}
void handleCastVoteEvents(SDL_Event* e, GUIState* guiState, SDL_Renderer* renderer, TTF_Font* font) {
    // Basic null checks
    if (!e || !guiState || !renderer || !font) return;

    // Change this to a local static variable with a larger buffer
    static char candidateID[32] = {0};  // Increased buffer size
    char voterID[16] = {0};
    static int inputFocus = 0;  // 0 for Voter ID, 1 for Candidate ID
    
    switch (e->type) {
        case SDL_MOUSEBUTTONDOWN:
            int x = e->button.x, y = e->button.y;

            // Voter ID box (focus)
            if (x >= 350 && x <= 650 && y >= 200 && y <= 250) {
                inputFocus = 0;  // Voter ID focus
            }

            // Candidate ID box (focus)
            if (x >= 350 && x <= 650 && y >= 300 && y <= 350) {
                inputFocus = 1;  // Candidate ID focus
            }

            // Back button
            if (x >= 350 && x <= 495 && y >= 400 && y <= 450) {
                guiState->currentScreen = 0;
                guiState->errorMessage[0] = '\0';
                guiState->inputBuffer[0] = '\0';
                candidateID[0] = '\0';  // Clear candidateID
                return;
            }

            // Submit button
            if (x >= 505 && x <= 650 && y >= 400 && y <= 450) {
                // Validate required pointers
                if (!guiState->voterTree || !guiState->bc) {
                    strcpy(guiState->errorMessage, "System error: Invalid data");
                    break;
                }

                // Safe string copy
                strncpy(voterID, guiState->inputBuffer, sizeof(voterID) - 1);
                voterID[sizeof(voterID) - 1] = '\0';

                // Validation checks
                if (strlen(voterID) == 0) {
                    strcpy(guiState->errorMessage, "Please enter a Voter ID");
                } else {
                    VoterNode *voter = findVoter(guiState->voterTree->root, voterID);
                    if (voter == NULL) {
                        strcpy(guiState->errorMessage, "Voter not registered");
                    } else if (voter->voted) {
                        strcpy(guiState->errorMessage, "Voter has already voted");
                    } else if (strlen(candidateID) == 0) {
                        strcpy(guiState->errorMessage, "Please enter a Candidate ID");
                    } else {
                        // Attempt to cast vote
                        castVote(voterID, candidateID, guiState->bc);

                        // Update voter status
                        int updateStatus = updateVotingStatus(guiState->voterTree->root, voterID);

                        if (updateStatus == 0) {
                            // Save updated voter tree
                            saveTreeToBinaryFile(guiState->voterTree, "voter_data.bin");

                            strcpy(guiState->errorMessage, "Vote cast successfully");
                            guiState->inputBuffer[0] = '\0';
                            candidateID[0] = '\0';
                        } else {
                            strcpy(guiState->errorMessage, "Error updating voter status");
                        }
                    }
                }
            }
            break;
        
        case SDL_TEXTINPUT:
            // Input based on current focus
            if (inputFocus == 0 && strlen(guiState->inputBuffer) < sizeof(guiState->inputBuffer) - 1) {
                strncat(guiState->inputBuffer, e->text.text, 
                        sizeof(guiState->inputBuffer) - strlen(guiState->inputBuffer) - 1);
            } else if (inputFocus == 1 && strlen(candidateID) < sizeof(candidateID) - 1) {
                strncat(candidateID, e->text.text, 
                        sizeof(candidateID) - strlen(candidateID) - 1);
                
                // Debug print to verify input
                printf("Candidate ID: %s\n", candidateID);
            }
            break;
        
        case SDL_KEYDOWN:
            if (e->key.keysym.sym == SDLK_BACKSPACE) {
                // Backspace based on current focus
                if (inputFocus == 0 && strlen(guiState->inputBuffer) > 0) {
                    guiState->inputBuffer[strlen(guiState->inputBuffer) - 1] = '\0';
                } else if (inputFocus == 1 && strlen(candidateID) > 0) {
                    candidateID[strlen(candidateID) - 1] = '\0';
                    
                    // Debug print to verify backspace
                    printf("Candidate ID after backspace: %s\n", candidateID);
                }
            }
            break;
    }
}
void renderCastVoteScreen(SDL_Renderer* renderer, TTF_Font* font, GUIState* guiState, const char* candidateIDParam) {
    // Null checks
    if (!renderer || !font || !guiState) return;

    // Clear the renderer with a white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Colors
    SDL_Color GRAY = {200, 200, 200, 255};
    SDL_Color BLACK = {0, 0, 0, 255};
    SDL_Color LIGHT_GRAY = {220, 220, 220, 255};
    SDL_Color RED = {255, 0, 0, 255};

    // Render labels and input boxes for Voter ID
    SDL_Rect voterIDRect = {350, 200, 300, 50};
    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 255);
    SDL_RenderFillRect(renderer, &voterIDRect);
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
    SDL_RenderDrawRect(renderer, &voterIDRect);

    // Safely render Voter ID label
    SDL_Surface* voterIDLabelSurface = TTF_RenderText_Blended(font, "Voter ID", BLACK);
    if (voterIDLabelSurface) {
        SDL_Texture* voterIDLabelTexture = SDL_CreateTextureFromSurface(renderer, voterIDLabelSurface);
        if (voterIDLabelTexture) {
            SDL_Rect voterIDLabelRect = {350, 170, voterIDLabelSurface->w, voterIDLabelSurface->h};
            SDL_RenderCopy(renderer, voterIDLabelTexture, NULL, &voterIDLabelRect);
            SDL_DestroyTexture(voterIDLabelTexture);
        }
        SDL_FreeSurface(voterIDLabelSurface);
    }

    // Render voter input
    SDL_Surface* voterInputSurface = TTF_RenderText_Blended(font, 
        guiState->inputBuffer ? guiState->inputBuffer : "", BLACK);
    if (voterInputSurface) {
        SDL_Texture* voterInputTexture = SDL_CreateTextureFromSurface(renderer, voterInputSurface);
        if (voterInputTexture) {
            SDL_Rect voterInputRect = {360, 210, voterInputSurface->w, voterInputSurface->h};
            SDL_RenderCopy(renderer, voterInputTexture, NULL, &voterInputRect);
            SDL_DestroyTexture(voterInputTexture);
        }
        SDL_FreeSurface(voterInputSurface);
    }

    // Render candidate ID input box
    SDL_Rect candidateIDRect = {350, 300, 300, 50};
    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 255);
    SDL_RenderFillRect(renderer, &candidateIDRect);
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
    SDL_RenderDrawRect(renderer, &candidateIDRect);

    // Safely render Candidate ID label
    SDL_Surface* candidateIDLabelSurface = TTF_RenderText_Blended(font, "Candidate ID", BLACK);
    if (candidateIDLabelSurface) {
        SDL_Texture* candidateIDLabelTexture = SDL_CreateTextureFromSurface(renderer, candidateIDLabelSurface);
        if (candidateIDLabelTexture) {
            SDL_Rect candidateIDLabelRect = {350, 270, candidateIDLabelSurface->w, candidateIDLabelSurface->h};
            SDL_RenderCopy(renderer, candidateIDLabelTexture, NULL, &candidateIDLabelRect);
            SDL_DestroyTexture(candidateIDLabelTexture);
        }
        SDL_FreeSurface(candidateIDLabelSurface);
    }

    // Render candidate input
    SDL_Surface* candidateInputSurface = TTF_RenderText_Blended(font, 
        (candidateIDParam && candidateIDParam[0] != '\0') ? candidateIDParam : "", BLACK);
    if (candidateInputSurface) {
        SDL_Texture* candidateInputTexture = SDL_CreateTextureFromSurface(renderer, candidateInputSurface);
        if (candidateInputTexture) {
            SDL_Rect candidateInputRect = {360, 310, candidateInputSurface->w, candidateInputSurface->h};
            SDL_RenderCopy(renderer, candidateInputTexture, NULL, &candidateInputRect);
            SDL_DestroyTexture(candidateInputTexture);
        }
        SDL_FreeSurface(candidateInputSurface);
    }

    // Back and Submit buttons
    SDL_Rect backButton = {350, 400, 145, 50};
    SDL_Rect submitButton = {505, 400, 145, 50};
    drawButton(renderer, font, "Back", &backButton, GRAY);
    drawButton(renderer, font, "Submit", &submitButton, GRAY);

    // Render error message
    if (guiState->errorMessage && strlen(guiState->errorMessage) > 0) {
        SDL_Surface* errorSurface = TTF_RenderText_Blended(font, guiState->errorMessage, RED);
        if (errorSurface) {
            SDL_Texture* errorTexture = SDL_CreateTextureFromSurface(renderer, errorSurface);
            if (errorTexture) {
                SDL_Rect errorRect = {350, 460, errorSurface->w, errorSurface->h};
                SDL_RenderCopy(renderer, errorTexture, NULL, &errorRect);
                SDL_DestroyTexture(errorTexture);
            }
            SDL_FreeSurface(errorSurface);
        }
    }

    SDL_RenderPresent(renderer);
}/*
void renderCastVoteScreen(SDL_Renderer* renderer, TTF_Font* font, GUIState* guiState, const char* candidateID) {
    // Null checks
    if (!renderer || !font || !guiState) return;

    // Clear the renderer with a white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Colors
    SDL_Color GRAY = {200, 200, 200, 255};
    SDL_Color BLACK = {0, 0, 0, 255};
    SDL_Color LIGHT_GRAY = {220, 220, 220, 255};
    SDL_Color RED = {255, 0, 0, 255};

    // Render labels and input boxes for Voter ID
    SDL_Rect voterIDRect = {350, 200, 300, 50};
    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 255);
    SDL_RenderFillRect(renderer, &voterIDRect);
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
    SDL_RenderDrawRect(renderer, &voterIDRect);

    // Safely render Voter ID label
    SDL_Surface* voterIDLabelSurface = TTF_RenderText_Blended(font, "Voter ID", BLACK);
    if (voterIDLabelSurface) {
        SDL_Texture* voterIDLabelTexture = SDL_CreateTextureFromSurface(renderer, voterIDLabelSurface);
        if (voterIDLabelTexture) {
            SDL_Rect voterIDLabelRect = {350, 170, voterIDLabelSurface->w, voterIDLabelSurface->h};
            SDL_RenderCopy(renderer, voterIDLabelTexture, NULL, &voterIDLabelRect);
            SDL_DestroyTexture(voterIDLabelTexture);
        }
        SDL_FreeSurface(voterIDLabelSurface);
    }

    // Render voter input
    SDL_Surface* voterInputSurface = TTF_RenderText_Blended(font, 
        guiState->inputBuffer ? guiState->inputBuffer : "", BLACK);
    if (voterInputSurface) {
        SDL_Texture* voterInputTexture = SDL_CreateTextureFromSurface(renderer, voterInputSurface);
        if (voterInputTexture) {
            SDL_Rect voterInputRect = {360, 210, voterInputSurface->w, voterInputSurface->h};
            SDL_RenderCopy(renderer, voterInputTexture, NULL, &voterInputRect);
            SDL_DestroyTexture(voterInputTexture);
        }
        SDL_FreeSurface(voterInputSurface);
    }

    // Render candidate ID input box
    SDL_Rect candidateIDRect = {350, 300, 300, 50};
    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, 255);
    SDL_RenderFillRect(renderer, &candidateIDRect);
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
    SDL_RenderDrawRect(renderer, &candidateIDRect);

    // Safely render Candidate ID label
    SDL_Surface* candidateIDLabelSurface = TTF_RenderText_Blended(font, "Candidate ID", BLACK);
    if (candidateIDLabelSurface) {
        SDL_Texture* candidateIDLabelTexture = SDL_CreateTextureFromSurface(renderer, candidateIDLabelSurface);
        if (candidateIDLabelTexture) {
            SDL_Rect candidateIDLabelRect = {350, 270, candidateIDLabelSurface->w, candidateIDLabelSurface->h};
            SDL_RenderCopy(renderer, candidateIDLabelTexture, NULL, &candidateIDLabelRect);
            SDL_DestroyTexture(candidateIDLabelTexture);
        }
        SDL_FreeSurface(candidateIDLabelSurface);
    }

    // Render candidate input - use the static candidateID instead of the passed parameter
     // Declare extern to access the static variable
    SDL_Surface* candidateInputSurface = TTF_RenderText_Blended(font, 
        candidateID ? candidateID : "", BLACK);
    if (candidateInputSurface) {
        SDL_Texture* candidateInputTexture = SDL_CreateTextureFromSurface(renderer, candidateInputSurface);
        if (candidateInputTexture) {
            SDL_Rect candidateInputRect = {360, 310, candidateInputSurface->w, candidateInputSurface->h};
            SDL_RenderCopy(renderer, candidateInputTexture, NULL, &candidateInputRect);
            SDL_DestroyTexture(candidateInputTexture);
        }
        SDL_FreeSurface(candidateInputSurface);
    }


    // Back and Submit buttons
    SDL_Rect backButton = {350, 400, 145, 50};
    SDL_Rect submitButton = {505, 400, 145, 50};
    drawButton(renderer, font, "Back", &backButton, GRAY);
    drawButton(renderer, font, "Submit", &submitButton, GRAY);

    // Render error message
    if (guiState->errorMessage && strlen(guiState->errorMessage) > 0) {
        SDL_Surface* errorSurface = TTF_RenderText_Blended(font, guiState->errorMessage, RED);
        if (errorSurface) {
            SDL_Texture* errorTexture = SDL_CreateTextureFromSurface(renderer, errorSurface);
            if (errorTexture) {
                SDL_Rect errorRect = {350, 460, errorSurface->w, errorSurface->h};
                SDL_RenderCopy(renderer, errorTexture, NULL, &errorRect);
                SDL_DestroyTexture(errorTexture);
            }
            SDL_FreeSurface(errorSurface);
        }
    }

    SDL_RenderPresent(renderer);
}*/

void handleCandidateManagementEvents(SDL_Event* e, GUIState* guiState) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_1: // Display Candidates
                displayCandidates();  // Call your logic for displaying candidates
                break;
            
            case SDLK_0: // Return to Main Menu
                guiState->currentScreen = 0;
                break;
            default:
                snprintf(guiState->errorMessage, sizeof(guiState->errorMessage),
                         "Invalid choice! Press 1 or 0.");
                break;
        }
    }
}
void renderCandidateManagement(SDL_Renderer* renderer, TTF_Font* font, GUIState* guiState) {
    if (!renderer || !font) {
        fprintf(stderr, "Renderer or font is null. Check initialization.\n");
        return;
    }

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White background
    SDL_RenderClear(renderer);

    // Text options (updated to remove the "Add Candidate" option)
    const char* options[] = {
        "=== Candidate Management ===",
        "1. Display Candidates",
        "0. Return to Main Menu",
        "Choose an option (Press 1 or 0):"
    };

    SDL_Color textColor = {0, 0, 0, 255};  // Black text
    int y_offset = 50;

    for (int i = 0; i < 4; ++i) {  // Updated loop to match the 4 options
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, options[i], textColor);
        if (!textSurface) {
            fprintf(stderr, "Text rendering failed: %s\n", TTF_GetError());
            continue;
        }

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (!textTexture) {
            fprintf(stderr, "Texture creation failed: %s\n", SDL_GetError());
            SDL_FreeSurface(textSurface);
            continue;
        }

        SDL_Rect textRect = {50, y_offset, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        y_offset += 50;
    }

    // Render error message
    if (strlen(guiState->errorMessage) > 0) {
        SDL_Surface* errorSurface = TTF_RenderText_Solid(font, guiState->errorMessage, textColor);
        if (errorSurface) {
            SDL_Texture* errorTexture = SDL_CreateTextureFromSurface(renderer, errorSurface);
            if (errorTexture) {
                SDL_Rect errorRect = {50, y_offset, errorSurface->w, errorSurface->h};
                SDL_RenderCopy(renderer, errorTexture, NULL, &errorRect);
                SDL_DestroyTexture(errorTexture);
            }
            SDL_FreeSurface(errorSurface);
        }
    }

    SDL_RenderPresent(renderer);
}
void renderVotingEndedScreen(SDL_Renderer* renderer, TTF_Font* font) {
    // Clear the screen with a background color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White background
    SDL_RenderClear(renderer);

    // Display message "Voting Period Ended"
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, "The Voting Period Has Ended!", (SDL_Color){255, 0, 0, 255});
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {100, 200, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    // Display additional message if needed, like a thank you note or instructions
    SDL_Surface* thankYouSurface = TTF_RenderText_Blended(font, "Thank you for participating!", (SDL_Color){0, 0, 0, 255});
    if (thankYouSurface) {
        SDL_Texture* thankYouTexture = SDL_CreateTextureFromSurface(renderer, thankYouSurface);
        SDL_Rect thankYouRect = {100, 300, thankYouSurface->w, thankYouSurface->h};
        SDL_RenderCopy(renderer, thankYouTexture, NULL, &thankYouRect);

        SDL_FreeSurface(thankYouSurface);
        SDL_DestroyTexture(thankYouTexture);
    }

    // Update the screen
    SDL_RenderPresent(renderer);
}

// Rest of the implementation would follow a similar pattern for other screens
// (Cast Vote, Alter Vote, Verify Blockchain, Count Votes)
int main(){
    // SDL Initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Blockchain Voting System", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
 time_t current_time = time(NULL);
    
    // Add 60 seconds (1 minute) to the current time to set expiration time
    time_t expiration_time = current_time + 30;

    // Open the file to write the expiration time
    FILE *file = fopen("voting_time.txt", "w");
    if (file != NULL) {
        fprintf(file, "%ld", expiration_time);  // Write the expiration time
        fclose(file);
        printf("Voting expiration time set to: %ld (Unix timestamp)\n", expiration_time);
    } else {
        printf("Error: Unable to open the file for writing.\n");
    }
    // Initialize blockchain and voter tree
    blockchain bc;
    AVLTree voterTree;
    
    bc.head = bc.tail = NULL;
    bc.merkle_count = 0;
    initializeTree(&voterTree);
    initializeBlockchain(&bc);

    // GUI State
    GUIState guiState = {
        .bc = &bc,
        .voterTree = &voterTree,
        .integrityFailed = 0,
        .errorMessage = {0},
        .inputBuffer = {0},
        .currentScreen = 0
    };

    // Main event loop
    SDL_Event e;
    int quit = 0;
    Candidate candidates[MAX_CANDIDATES];
    int numCandidates;
    loadCandidatesFromFile(candidates,&numCandidates);
    while (!quit) {
        // Check voting time at the start of each loop iteration
        if (hasTimePassed("voting_time.txt")) {
            renderVotingEndedScreen(renderer, font);
            countVotes(&bc, candidates, numCandidates); 
            SDL_Delay(3000);  // Show the message for 3 seconds
            quit = 1;
            break;  // Exit the loop immediately
        }

        while (SDL_PollEvent(&e) != 0) {
            // Quit event
            if (e.type == SDL_QUIT) {
                quit = 1;
            }

            // Handle events based on current screen
            switch (guiState.currentScreen) {
                case 0: // Main Menu
                    handleMainMenuEvents(&e, &guiState);
                    break;
                case 1: // Register Voter
                    handleRegisterVoterEvents(&e, &guiState, renderer, font);
                    break;
                case 2: // Cast Vote
                    handleCastVoteEvents(&e, &guiState, renderer, font);
                    break;
                case 3: // Candidate Management
                    handleCandidateManagementEvents(&e, &guiState);
                    break;
            }
        }

        // Render based on current screen
        switch (guiState.currentScreen) {
            case 0:
                renderMainMenu(renderer, font, &guiState);
                break;
            case 1:
                renderRegisterVoter(renderer, font, &guiState);
                break;
            case 2:
                renderCastVoteScreen(renderer, font, &guiState, candidateID);
                break;
            case 3:
                renderCandidateManagement(renderer, font, &guiState);
                break;
        }
 
        // Count votes if voting is still ongoing and integrity is maintained
       /* if (guiState.integrityFailed == 0) {
            countVotes(&bc, candidates, numCandidates);
        }*/

        SDL_Delay(16);  // Cap at ~60 FPS
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}


/*
int main() {
    // SDL Initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Blockchain Voting System", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Initialize blockchain and voter tree
    blockchain bc;
    AVLTree voterTree;
    
    bc.head = bc.tail = NULL;
    bc.merkle_count = 0;
    initializeTree(&voterTree);
    initializeBlockchain(&bc);

    // GUI State
    GUIState guiState = {
        .bc = &bc,
        .voterTree = &voterTree,
        .integrityFailed = 0,
        .errorMessage = {0},
        .inputBuffer = {0},
        .currentScreen = 0
    };

    // Main event loop
    SDL_Event e;
    int quit = 0;

    while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = 1;
        }

        // Screen-based event handling
        switch (guiState.currentScreen) {
            case 0:
                handleMainMenuEvents(&e, &guiState);
                break;
            case 1:
                handleRegisterVoterEvents(&e, &guiState, renderer, font);
                break;
            case 2:
                handleCastVoteEvents(&e, &guiState, renderer, font);
                break;
                 case 3:  // Candidate Management Screen
                handleCandidateManagementEvents(&e, &guiState, renderer, font);
                 break;
            default:
                printf("Error: Unknown screen state %d\n", guiState.currentScreen);
                quit = 1;
                break;
        }
    }

    // Screen-based rendering
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White background
    SDL_RenderClear(renderer);

    switch (guiState.currentScreen) {
        case 0:
            renderMainMenu(renderer, font, &guiState);
            break;
        case 1:
            renderRegisterVoter(renderer, font, &guiState);
            break;
        case 2:
            renderCastVoteScreen(renderer, font, &guiState, candidateID);
            break;
            case 3:  // Candidate Management Screen
            renderCandidateManagement(renderer, font, &guiState);
            break;
        default:
            break;
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);  // ~60 FPS
}


    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}*/

// Main menu event handler
void handleMainMenuEvents(SDL_Event* e, GUIState* guiState) {
    if (e->type == SDL_MOUSEBUTTONDOWN) {
        int x = e->button.x, y = e->button.y;

        // Register Voter Button
        if (x >= 350 && x <= 650 && y >= 200 && y <= 250) {
            guiState->currentScreen = 1;  // Go to Register Voter screen
            guiState->errorMessage[0] = '\0';  // Clear any error message
        }
        // Cast Vote Button
        else if (x >= 350 && x <= 650 && y >= 270 && y <= 320) {
            guiState->currentScreen = 2;  // Go to Cast Vote screen
            guiState->errorMessage[0] = '\0';  // Clear any error message
        }
        // Candidate Management Button
        else if (x >= 350 && x <= 650 && y >= 340 && y <= 390) {
            guiState->currentScreen = 3;  // Go to Candidate Management screen
            guiState->errorMessage[0] = '\0';  // Clear any error message
        }
       
        // Exit Button
        else if (x >= 350 && x <= 650 && y >= 550 && y <= 600) {
            SDL_Event quitEvent;
            quitEvent.type = SDL_QUIT;
            SDL_PushEvent(&quitEvent);  // Trigger quit event
        }
    }
}


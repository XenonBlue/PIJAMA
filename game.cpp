#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_opengl.h>

#include "display.cpp"

class Game
{
	protected:
	ALLEGRO_EVENT_QUEUE		*eventQueue;
	GameDisplay				*gameDisplay;
	GameInput				*gameInput;

	Player					*p1;
	Bot						*f_bot1;
	EnemyBot				*e_bot;

	bool					render;
	bool					exitGame;

	public:
	Game()
	{
		this->gameDisplay = NULL;
		this->gameInput = NULL;
		this->eventQueue = NULL;

		this->p1 = NULL;
		this->f_bot1 = NULL;
		this->e_bot = NULL;

		this->render = false;
		this->exitGame = false;
	}

	~Game()
	{
		delete e_bot;
		delete f_bot1;
		delete p1;
		delete gameInput;
		delete gameDisplay;

		al_shutdown_native_dialog_addon();
		al_destroy_event_queue(eventQueue);
	}

	bool initAllegro()
	{
		if (!al_init())
			return false;

		al_install_keyboard();
		al_install_mouse();
		al_install_audio();
		al_init_acodec_addon();
		al_init_primitives_addon();
		al_init_image_addon();
		al_init_font_addon();
		al_init_ttf_addon();
		al_init_native_dialog_addon();

		return true;
	}

	bool init()
	{
		if (!this->initAllegro())
			return false;

		this->gameDisplay = new GameDisplay();
		if (!gameDisplay->getDisplay())
			return false;

		this->gameInput = new GameInput();

		this->p1 = new Player(gameInput, 800, 800, 100.);
		this->f_bot1 = new Bot(800, 850, 100.);
		this->e_bot = new EnemyBot(1280, 1440, 100.);

		p1->setFont(gameDisplay->getFont(1));
		f_bot1->setFont(gameDisplay->getFont(1));
		e_bot->setFont(gameDisplay->getFont(1));

		eventQueue = al_create_event_queue();
		if (!eventQueue)
			return false;

		al_register_event_source(eventQueue, al_get_timer_event_source(gameDisplay->getTimerFPS()));
		al_register_event_source(eventQueue, al_get_timer_event_source(gameDisplay->getTimerAnimation()));
		al_register_event_source(eventQueue, al_get_display_event_source(gameDisplay->getDisplay()));
		al_register_event_source(eventQueue, al_get_mouse_event_source());
		al_register_event_source(eventQueue, al_get_keyboard_event_source());

		return true;
	}

	void run()
	{
		this->gameDisplay->startSFX();
		this->gameDisplay->startTimerAnimation();
		this->gameDisplay->startTimerFPS();

		while(!this->exitGame)
		{
			ALLEGRO_EVENT event;
//			ALLEGRO_TIMEOUT timeout;
//			al_init_timeout(&timeout, 0.06);
//			bool getEvent = al_wait_for_event_until(eventQueue, &event, &timeout);
			al_wait_for_event(eventQueue, &event);

			if (event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				this->p1->input(event.keyboard.keycode, true);
			}
			else if (event.type == ALLEGRO_EVENT_KEY_UP)
			{
				this->p1->input(event.keyboard.keycode, false);
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
			{

			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
			{

			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_AXES ||
					 event.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY)
			{
				//p1->setX(event.mouse.x);
				//p1->setY(event.mouse.y);
				//gameDisplay->setScale(event.mouse.dz/10.);
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			{
				this->exitGame = true;
			}
			else if (event.type == ALLEGRO_EVENT_TIMER)
			{
				if (event.timer.source == this->gameDisplay->getTimerFPS())
				{
					this->gameDisplay->updateFPS();

					if (this->gameInput->keyDown(ALLEGRO_KEY_ESCAPE))
						this->exitGame = true;

					this->p1->update(this->f_bot1);
					this->f_bot1->update(this->e_bot, this->p1);

					if (this->e_bot->getDistance(this->f_bot1) < this->e_bot->getDistance(this->p1))
						this->e_bot->update(this->f_bot1);
					else
						this->e_bot->update(this->p1);

					this->gameDisplay->updateCamera(this->p1);
				}
				else if (event.timer.source == this->gameDisplay->getTimerAnimation())
				{
					this->p1->updateAnimation();
					this->f_bot1->updateAnimation();
					this->e_bot->updateAnimation();
				}

				this->render = true;
			}

			if (render && al_is_event_queue_empty(eventQueue))
			{
				this->render = false;

				this->p1->render();
				this->f_bot1->render();
				this->e_bot->render();

				this->gameDisplay->draw();
			}
		}
	}

	void changeState(int &state, int newState)
	{
		state = newState;
	}

	ALLEGRO_BITMAP*	createBitmapFromLayers(ALLEGRO_BITMAP *texture[], int n)
	{
		ALLEGRO_BITMAP *bmp = al_create_bitmap(al_get_bitmap_width(texture[0]), al_get_bitmap_height(texture[0]));
		al_set_target_bitmap(bmp);

		for (int i = 0; i < n; i++)
			al_draw_bitmap(texture[i], 0, 0, 0);

		al_save_bitmap("gfx/bmp/bitmap.png", bmp);
		al_set_target_bitmap(al_get_backbuffer(this->gameDisplay->getDisplay()));

		return bmp;
	}

	int showMessageBox(const char *text, int flag, const char *button = NULL)
	{
		switch (flag)
		{
		case ALLEGRO_MESSAGEBOX_WARN:
			return al_show_native_message_box(DISPLAY, "Attenzione", "Potrebbe esserci un problema!", text, button, ALLEGRO_MESSAGEBOX_WARN);
		case ALLEGRO_MESSAGEBOX_ERROR:
			return al_show_native_message_box(DISPLAY, "Errore Allegro 5", "Errore inizializzazione!", text, button, ALLEGRO_MESSAGEBOX_ERROR);
		case ALLEGRO_MESSAGEBOX_QUESTION:
			return al_show_native_message_box(DISPLAY, "Domanda", "Scegliere come procedere.", text, button, ALLEGRO_MESSAGEBOX_QUESTION);
		case ALLEGRO_MESSAGEBOX_OK_CANCEL:
			return al_show_native_message_box(DISPLAY, "Ok / Cancella", "Cosa vuoi fare?", text, button, ALLEGRO_MESSAGEBOX_OK_CANCEL);
		case ALLEGRO_MESSAGEBOX_YES_NO:
			return al_show_native_message_box(DISPLAY, "Si / No", "Scelta.", text, button, ALLEGRO_MESSAGEBOX_YES_NO);
		default:
			return al_show_native_message_box(DISPLAY, "Errore", "Errore del programma", text, button, ALLEGRO_MESSAGEBOX_ERROR);
		}

		return -1;																	// -1: Errore // 0: Chiusura // 1: Ok/Yes // 2: Cancel/No
	}
};
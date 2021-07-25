{*--------------------------------------------------------------------------*}
{*                            T I T L E . I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Game title scene                                            *}
{* Prequisites: GLOBALS.INC, Core game engine                               *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __PLUTONIUM_CAVERNS_TITLE__}
{$define __PLUTONIUM_CAVERNS_TITLE__}

const	TITLE_LOGO_TIME	= 30;

var	title_logo_loaded	: boolean;
	title_logo_drawn	: boolean;
        title_bg_drawn		: boolean;
        title_story_played	: boolean;

        title_logo_timer	: int16;
        title_enter_timer	: int8;
        title_phase		: uint8;

        bmp_logo		: BITMAP;
        bmp_font		: BITMAP;
        bmp_smenu		: BITMAP;

	title_menu		: MENU;
        title_confirm_active	: boolean;
        title_confirm_cursor	: boolean;
        title_confirm_changed	: boolean;

function	title_init: boolean;                    forward;
procedure	title_update(steps: int16);             forward;
procedure	title_draw;                             forward;
procedure	title_change(param: pointer);           forward;
procedure	title_dispose;                          forward;

procedure	title_start_game;
begin
	{ clear data }
	AST_remove('logo');
        title_logo_loaded := FALSE;

        { load game assets }
        if not game_load_assets then
        begin
                APP_terminate;
        	exit;
        end;

        { reinitialized }
	if title_story_played then smenu_init_assets;

        { go to the story scene if story already played }
        if title_story_played then
	        app_change_scene(SCENE_SMENU, pointer(int16(0)))
        else	app_change_scene(SCENE_STORY, pointer(int16(0)));

        title_story_played := TRUE;
end;

procedure	title_cb_start;
begin
	TRN_activate(FADE_IN, 2, title_start_game);
end;

procedure	title_cb_audio;
var	audio_enabled	: boolean;
begin
        { toggle sound and update the menu text }
	audio_enabled := SND_is_enabled;
        SND_toggle;
	if not audio_enabled then
        begin
        	title_menu.text[1, 10] := 'N';
                title_menu.text[1, 11] := ' ';
        end
        else
        begin
        	title_menu.text[1, 10] := 'F';
                title_menu.text[1, 11] := 'F';
        end;
        title_menu.redraw := TRUE;
end;

procedure	title_cb_clear;
begin
	title_confirm_active	:= TRUE;
	title_confirm_changed	:= TRUE;
        title_confirm_cursor	:= TRUE;
end;

procedure	title_cb_quit;
begin
        TRN_activate(FADE_IN, 2, APP_terminate);
end;

procedure       title_create_pause_menu;
begin
	{ create main menu }
	MNU_create(title_menu, 3);

        { add buttons }
        MNU_add_button(title_menu, 'START GAME', title_cb_start);
        MNU_add_button(title_menu, 'AUDIO: ON ', title_cb_audio);
        MNU_add_button(title_menu, 'CLEAR DATA', title_cb_clear);
        MNU_add_button(title_menu, 'QUIT GAME ', title_cb_quit);
end;

{ draw stars }
procedure	title_draw_stars(i, x, y: uint8);
begin
	VGA_draw_bitmap_region_fast(bmp_smenu, 64,
		16 + i * 16, 16, 16, x * 16, y * 16);
end;

{ draw background }
procedure	title_draw_background;
const	PLANET_X	= 320-80;
	PLANET_Y	= 96;
begin
	VGA_clear(0);

        { draw planet }
        VGA_draw_bitmap_region_fast(bmp_smenu, 0, 0, 64, 64,
		PLANET_X, PLANET_Y);

        { draw stars at random position }
        title_draw_stars(0, 2, 4);
        title_draw_stars(1, 1, 2);
        title_draw_stars(0, 1, 6);
        title_draw_stars(2, 4, 7);
        title_draw_stars(1, 1, 9);
        title_draw_stars(0, 16, 3);
        title_draw_stars(1, 19, 1);
        title_draw_stars(2, 18, 5);
end;

{ draw confirm screen }
procedure	title_draw_confirm_screen;
const	WIDTH	= 128;
	HEIGHT	= 24;
var	x, y	: int16;
begin
	x := 160 - WIDTH shr 1;
        y := 100 - HEIGHT shr 1;

        if title_confirm_changed then
        begin
        	VGA_fill_rect(x-2, y-2, WIDTH+4, HEIGHT+4, 146);
                VGA_fill_rect(x-1, y-1, WIDTH+2, HEIGHT+2, 255);
                VGA_fill_rect(x, y, WIDTH, HEIGHT, 0);

                VGA_draw_text_fast(bmp_font, 'ARE YOU SURE?',
			x+WIDTH shr 1, y + 3, 0, 0, TRUE);
        end;

        { yes/no text }
        if title_confirm_cursor = FALSE then
	        VGA_draw_text_fast(bmp_font, #6'YES    NO'  , x+WIDTH shr 1, y+HEIGHT shr 1 + 1, 0, 0, TRUE)
        else	VGA_draw_text_fast(bmp_font, ' YES   '#6'NO', x+WIDTH shr 1, y+HEIGHT shr 1 + 1, 0, 0, TRUE)
end;

function	title_init: boolean;
begin
	title_init := FALSE;

        { load global stuffs }
        if not AST_add_bitmap(F_BMP_FONT,  'font') or
           not AST_add_bitmap(F_BMP_SMENU, 'smenu') then exit;

        bmp_font  := AST_get('font');
        bmp_smenu := AST_get('smenu');

        { pass data to global components }
        MNU_init;

        { defaults }
        title_logo_loaded     := FALSE;
        title_logo_drawn      := FALSE;
        title_bg_drawn        := FALSE;
        title_enter_timer     := 59;
        title_phase           := 0;
        title_logo_timer      := TITLE_LOGO_TIME;
        title_confirm_active  := FALSE;
        title_confirm_changed := FALSE;
        title_confirm_cursor  := TRUE;

        { check if save file exist }
        title_story_played    := smenu_load_file;

        title_create_pause_menu;
        { create defaults }

	title_init := TRUE;
end;

procedure	title_update(steps: int16);
const	LOGO_SPEED	= 2;
begin
        { if confirm menu active }
        if title_confirm_active then
        begin
        	if (KBD_get_arrow_key(ARROW_LEFT) = KBD_PRESSED) or
                   (KBD_get_arrow_key(ARROW_RIGHT) = KBD_PRESSED) then
                begin
                	title_confirm_cursor := not title_confirm_cursor;
                        SND_play(SND_BEEP2);
                end;

                { ENTER ? }
        	if KBD_get_button(2) = KBD_PRESSED then
	        begin
        		if title_confirm_cursor = FALSE then
	                begin
        	        	smenu_clear_progress;
	                        title_story_played := FALSE;
        	                SND_play(SND_BEEP3);
	                end
	                else	SND_play(SND_BEEP4);

        	        title_confirm_active := FALSE;
	                title_bg_drawn       := FALSE;
        	        title_menu.redraw    := TRUE;
	                title_logo_drawn     := FALSE;
	        end;
		exit;
        end;

        { quit ? }
        if KBD_get_button(3) = KBD_PRESSED then
        begin
        	SND_play(SND_BEEP4);
                TRN_activate(FADE_IN, 2, APP_terminate);
                exit;
        end;

        if TRN_is_active then
        begin
        	{ update logo timer }
        	dec(title_logo_timer, LOGO_SPEED * steps);

                if title_logo_timer < 0 then
                begin
                	title_logo_timer := 0;
                        title_logo_drawn := FALSE;
                end;
                exit;
        end
        else
        begin
        	if title_logo_timer > 0 then
                begin
                	title_logo_timer := 0;
                        title_logo_drawn := FALSE;
                end;
        end;

        if title_phase = 0 then
        begin
        	{ update 'press enter' message timer }
        	inc(title_enter_timer, steps);
		if title_enter_timer >= 60 then
			dec(title_enter_timer, 60);

                { check for ENTER key }
                if KBD_get_button(2) = KBD_PRESSED then
                begin
                	inc(title_phase);
                        MNU_activate(title_menu, 0);
                        SND_play(SND_BEEP1);
                end;
        end
        else	if title_phase = 1 then MNU_update(title_menu, steps);
end;

procedure	title_draw;
const	LOGO_Y		= 16;
	COPYRIGHT_Y	= -10;
        ENTER_Y		= 144;
        MENU_Y		= 136;
begin
	VGA_toggle_clipping(TRUE);

        if not title_bg_drawn then
        begin
        	title_draw_background;
                title_bg_drawn := TRUE;
        end;

        if not title_logo_drawn or (title_logo_timer > 0) then
        begin
        	{ draw logo }
        	VGA_fill_rect(160-bmp_logo^.w shr 1, LOGO_Y-title_logo_timer-16,
			bmp_logo^.w, 16, 0);
                VGA_draw_bitmap_fast(bmp_logo,
			160-bmp_logo^.w div 2, LOGO_Y-title_logo_timer);

                { draw copyright }
                VGA_draw_text_fast(bmp_font, #4' 2019 JANI NYK'#5'NEN',
                	160, 200 + COPYRIGHT_Y, 0, 0, TRUE);
                title_logo_drawn := TRUE;
        end;

        { draw 'press enter' }
        if title_phase = 0 then
        begin
        	if title_enter_timer < 30 then
                	VGA_draw_text_fast(bmp_font, 'PRESS ENTER',
				160, ENTER_Y, 0, 0, TRUE)
                else	VGA_fill_rect(160- 8 * 6, ENTER_Y - 8, 16 * 6, 16, 0);
        end
        else    { draw menu }
		MNU_draw(title_menu, 160, MENU_Y);

        { draw confirm box }
        if title_confirm_active then title_draw_confirm_screen;
end;

procedure	title_dispose;
begin
	{ nothing to do here }
end;

procedure	title_change(param: pointer);
begin
	{ remove all game assets }
	game_clear_assets;

        { load own assets }
	if not title_logo_loaded then
        begin
        	{ logo }
        	if not AST_add_bitmap(F_BMP_LOGO, 'logo') then
                begin
                	APP_terminate;
                        exit;
                end;
		bmp_logo := AST_get('logo');
                bmp_font := AST_get('font');
        end;
        title_logo_drawn  := FALSE;
        title_logo_loaded := FALSE;
        title_bg_drawn    := FALSE;
        title_menu.redraw := TRUE;

        { set audio button text }
        SND_toggle;
        title_cb_audio;
end;

function	title_setup_scene: SCENE;
var	s	: SCENE;
begin
	new(s);
        with s^ do
        begin
        	init    := title_init;
        	update  := title_update;
        	draw    := title_draw;
        	dispose := title_dispose;
        	change  := title_change;
        	name    := SCENE_TITLE;
	end;
        title_setup_scene := s;
end;

{$endif}

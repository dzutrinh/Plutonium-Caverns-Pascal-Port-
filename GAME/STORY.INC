{*--------------------------------------------------------------------------*}
{*                            S T O R Y . I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Game story scene                                            *}
{* Prequisites: GLOBALS.INC, Core game engine                               *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __PLUTONIUM_CAVERNS_STORY__}
{$define __PLUTONIUM_CAVERNS_STORY__}

const	STORY_LETTER_TIME	= 3;

var	story_bmp	: BITMAP;
	story_font	: BITMAP;
        story_pointer	: uint8;
        story_bg_drawn	: boolean;
        story_chr_timer	: int16;
        story_chr_pos	: int16;
        story_len	: uint16;

function	story_init: boolean;                    forward;
procedure	story_update(steps: int16);             forward;
procedure	story_draw;                             forward;
procedure	story_change(param: pointer);           forward;
procedure	story_dispose;                          forward;

const	STORY_TEXT	: array[0..1] of string =
	(('YOU ARE AN ALIEN SPACE TRAVELER, FAR'#13+
	  'AWAY FROM HOME. WHILE EXPLORING SPACE,'#13+
	  'YOU RAN OUT OF FUEL AND HAD TO LAND'#13+
	  'ON A MYSTERIOUS PLANET. YOU MUST'#13+
	  'EXPLORE DARK CAVERNS AND COLLECT'#13+
	  'ENOUGH PLUTONIUM GEMS TO FUEL UP'#13+
	  'YOUR SHIP AND RETURN TO YOUR HOME'#13+
	  'PLANET.')
	  ,
         ('WITH THE POWER OF PLUTONIUM'#13+
	  'YOU FUEL YOU SHIP AND ESCAPE THIS'#13+
	  'COLD, COLD PLANET. NOW YOU CAN'#13+
	  'FINALLY RETURN YOUR HOME, A PLANET'#13+
	  'CALLED... EARTH.'#13));

procedure       story_cb_goto_stage;
begin
	{ remove picture }
        VGA_destroy_bitmap(story_bmp);

        { reinitialize assets }
        smenu_init_assets;

        { change scene }
        if story_pointer = 0 then
	        app_change_scene(SCENE_SMENU, pointer(0))
        else	app_change_scene(SCENE_INTRO, pointer(0));
end;

function	story_init: boolean;
begin
        { get fonts }
	story_font      := AST_get('font');

        { set defaults }
        story_bg_drawn  := FALSE;
        story_chr_timer := STORY_LETTER_TIME;
        story_chr_pos   := 0;
	story_pointer   := 0;
        story_len	:= length(STORY_TEXT[story_pointer]);
        story_init	:= TRUE;
end;

procedure	story_update(steps: int16);
begin
        if TRN_is_active then exit;

        { ENTER or ESC pressed ? }
        if (KBD_get_button(2) = KBD_PRESSED) or
           (KBD_get_button(3) = KBD_PRESSED) then
        begin
                TRN_activate(FADE_IN, 2, story_cb_goto_stage);
                SND_play(SND_BEEP3);
        end;

        { update character timer }
        dec(story_chr_timer, steps);
        if (story_chr_pos <= story_len) and
	   (story_chr_timer <= 0) then
        begin
        	inc(story_chr_timer, STORY_LETTER_TIME);
                inc(story_chr_pos);
        end;
end;

procedure	story_draw;
const	PICTURE_Y	= 16;
	STORY_Y		= 96;
        STORY_X		= 8;
begin
	{ draw background }
	if not story_bg_drawn then
        begin
                VGA_clear(0);
                VGA_draw_bitmap_fast(story_bmp,
			160-story_bmp^.w shr 1, PICTURE_Y);
		story_bg_drawn := TRUE;
        end;

        if not TRN_is_active then
        	{ draw text }
                VGA_draw_substr_fast(story_font,
			STORY_TEXT[story_pointer],
			STORY_X, STORY_Y, 0, 1,
			story_chr_pos, story_chr_pos + 1, FALSE);
end;

procedure	story_dispose;
begin
	{ nothing to do here }
end;

procedure	story_change(param: pointer);
begin
	{ load intro bitmap }
	if uint8(param) = 0 then
	        story_bmp := VGA_load_bitmap(F_BMP_STORY1)
        else	story_bmp := VGA_load_bitmap(F_BMP_STORY2);

        if story_bmp = nil then app_terminate;

        { reset values }
        story_bg_drawn  := FALSE;
        story_chr_timer := STORY_LETTER_TIME;
        story_chr_pos   := 0;
        story_pointer   := uint8(param);
end;

function	story_setup_scene: SCENE;
var	s	: SCENE;
begin
	new(s);
        with s^ do
        begin
        	init    := story_init;
        	update  := story_update;
        	draw    := story_draw;
        	dispose := story_dispose;
        	change  := story_change;
        	name    := SCENE_STORY;
	end;
        story_setup_scene := s;
end;

{$endif}

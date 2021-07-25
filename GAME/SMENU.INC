{*--------------------------------------------------------------------------*}
{*                             S M E N U. I N C                             *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Stage selection menu scene                                  *}
{* Prequisites: GLOBALS.INC, Core game engine                               *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __PLUTONIUM_CAVERNS_SMENU__}
{$define __PLUTONIUM_CAVERNS_SMENU__}

const   SMENU_STAGES	= 17;
        SMENU_ANIM_TIME	= 120;
        SMENU_PLANET_X	= 320 - 64;
        SMENU_PLANET_Y	= 100;

var     smenu_redraw_bg		: boolean;
	smenu_redraw_menu	: boolean;
        smenu_fill_oldcurpos	: boolean;
	smenu_pcpos		: VEC2;
        smenu_cpos		: VEC2;
        smenu_opos		: VEC2;
        smenu_anim_timer	: int16;
        smenu_stage_index	: uint8;
        smenu_stage_target	: uint8;
        smenu_bmp		: BITMAP;
        smenu_font		: BITMAP;

function	smenu_init: boolean;                    forward;
procedure	smenu_update(steps: int16);             forward;
procedure	smenu_draw;                             forward;
procedure	smenu_change(param: pointer);           forward;
procedure	smenu_dispose;                          forward;

procedure	smenu_draw_stars(i, x, y: uint8);
begin
        VGA_draw_bitmap_region_fast(smenu_bmp, 64,
		16 + i * 16, 16, 16, x * 16, y  * 16);
end;

procedure	smenu_save_data;
var	f	: FILE;
begin
        if file_open(f, TRUE, F_SAVE_GAME) then
        begin
	        file_write(smenu_stage_index, sizeof(uint8), f);
        	file_close(f);
        end;
end;

procedure	smenu_draw_bg;
begin
        VGA_clear(0);

        { draw stars to random positions }
        smenu_draw_stars(0, 12, 4);
        smenu_draw_stars(1, 14, 2);
        smenu_draw_stars(2, 18, 3);
        smenu_draw_stars(1, 19, 1);

        smenu_draw_stars(2, 13, 9);
        smenu_draw_stars(0, 16, 11);
        smenu_draw_stars(1, 19, 7);
        smenu_draw_stars(0, 18, 8);

        { draw planet cursor }
        VGA_draw_bitmap_region(smenu_bmp, 64, 0, 16, 16,
		smenu_pcpos.x-8, smenu_pcpos.y-8, FALSE);
end;

procedure	smenu_draw_planet;
var	animpos	: int16;
begin
	{ draw planet }
	VGA_draw_bitmap_region_fast(smenu_bmp, 0, 0, 64, 64,
		SMENU_PLANET_X - 32, SMENU_PLANET_Y - 32);

        { draw marker }
        VGA_draw_bitmap_region(smenu_bmp, 64, 0, 16, 16,
		smenu_pcpos.x - 8, smenu_pcpos.y - 8, FALSE);

        { draw cursor }
        animpos := SMENU_ANIM_TIME shr 1 -
		   abs(smenu_anim_timer - SMENU_ANIM_TIME shr 1);
        animpos := animpos div (SMENU_ANIM_TIME shr 3);
        VGA_draw_bitmap_region(smenu_bmp, 80, 16, 16, 16,
		smenu_pcpos.x - 8, smenu_pcpos.y - 20 - animpos, FALSE);
end;

procedure	smenu_draw_cursor(dx, dy, xoff, yoff: int16);
var	animpos	: int16;
begin
	animpos := abs(SMENU_ANIM_TIMER - SMENU_ANIM_TIME shr 1);
        animpos := animpos div (SMENU_ANIM_TIME shr 3);

        { hide old cursor }
        if smenu_fill_oldcurpos then
        begin
                VGA_fill_rect(	dx - 20 + smenu_opos.x * xoff,
				dy - 2  + smenu_opos.y * yoff,
				20, 16, 0);
                smenu_fill_oldcurpos := FALSE;
        end;

        { draw cursor }
        VGA_draw_bitmap_region_fast(smenu_bmp, 80, 0, 16, 16,
        	dx - 20 + animpos + smenu_cpos.x * xoff,
                dy - 2  + smenu_cpos.y * yoff);
end;

procedure	smenu_draw_menu;
const	HEADER_X	= 96;
	HEADER_Y	= 16;
        BOX_W		= 128;
        BOX_H		= 16;
        BUTTON_X	= 24;
        BUTTON_Y	= 32;
        BUTTON_OFF_X	= 96;
        BUTTON_OFF_Y	= 16;
var	i, m, bx, by	: uint16;
	st, buf		: string[10];
begin
        m  := (SMENU_STAGES+1) shr 1;
        bx := HEADER_X - 64;
        by := HEADER_Y - 4;

        { draw menu }
        if smenu_redraw_menu then
        begin
        	{ header box }
        	VGA_fill_rect(bx-2, by-2, BOX_W+4, BOX_H+4, 146);
                VGA_fill_rect(bx-1, by-1, BOX_W+2, BOX_H+2, 255);
                VGA_fill_rect(bx, by, BOX_W, BOX_H, 0);

                { header text }
                VGA_draw_text_fast(smenu_font, 'CHOOSE A STAGE',
			HEADER_X, HEADER_Y, 0, 0, TRUE);

                { stage buttons }
                for i := 0 to SMENU_STAGES do
                begin
                	if i > smenu_stage_index then
                        	buf := 'LOCKED'
                        else	buf := 'STAGE '+ltoa(i);

                        if i = 0 then st := 'BACK' else st := buf;
                        VGA_draw_text_fast(smenu_font, st,
                        	BUTTON_X + (i div m) * BUTTON_OFF_X,
                                HEADER_Y + BUTTON_Y + BUTTON_OFF_Y * (i mod m),
                                0, 0, FALSE);
                end;
                smenu_redraw_menu := FALSE;
        end;
        { draw menu cursor }
        smenu_draw_cursor(BUTTON_X, HEADER_Y + BUTTON_Y,
		BUTTON_OFF_X, BUTTON_OFF_Y);
end;

{ calculate planet cursor position }
procedure	smenu_compute_pcursor_pos;
const   RADIUS	= 11;
var	i	: int16;
begin
	if (smenu_cpos.x = 0) and (smenu_cpos.y = 0) then exit;

        i := smenu_cpos.x * (SMENU_STAGES+1) shr 1 + smenu_cpos.y;

        randseed := i;
        smenu_pcpos.x := SMENU_PLANET_X + random(RADIUS * 2) - RADIUS;
        smenu_pcpos.y := SMENU_PLANET_Y + random(RADIUS * 2) - RADIUS + 1;
end;

function	smenu_init: boolean;
begin
	{ set defaults }
	smenu_redraw_bg	  := TRUE;
        smenu_redraw_menu := TRUE;
        smenu_anim_timer  := 0;
        smenu_stage_index := 1;

        { cursor position }
	smenu_pcpos.x := 320 - 96 + 16;
        smenu_pcpos.y := 100 - 32 + 24;
        smenu_cpos.x  := 0;
        smenu_cpos.y  := 1;
        smenu_compute_pcursor_pos;

        smenu_init    := TRUE;
end;

{ go to stage }
procedure	smenu_cb_goto_stage;
begin
	if smenu_stage_target <> 0 then
                app_change_scene(SCENE_GAME, pointer(uint8(smenu_stage_target)))
        else	app_change_scene(SCENE_TITLE, nil);
end;

{ ESC key hit }
procedure	smenu_cb_escape;
begin
	app_change_scene(SCENE_TITLE, nil);
end;

procedure	smenu_update(steps: int16);
const	ANIM_SPEED	= 1;
begin
	smenu_stage_target := smenu_cpos.x * ((SMENU_STAGES+1) shr 1) +
			      smenu_cpos.y;

        { temporary quit }
        if KBD_get_button(3) = KBD_PRESSED then
        begin
        	SND_play(SND_BEEP4);
                TRN_activate(FADE_IN, 2, smenu_cb_escape);
                exit;
        end;

        if TRN_active then exit;

        { ENTER pressed ? }
        if (KBD_get_button(2) = KBD_PRESSED) and
           (smenu_stage_target <= smenu_stage_index) then
        begin
        	SND_play(SND_BEEP3);
                TRN_activate(FADE_IN, 2, smenu_cb_goto_stage);
                exit;
        end;

        { animate cursor }
        inc(smenu_anim_timer, ANIM_SPEED * steps);
	smenu_anim_timer := smenu_anim_timer mod SMENU_ANIM_TIME;

        { update cursor position }
        smenu_opos := smenu_cpos;

        { vertical }
        if KBD_get_arrow_key(ARROW_DOWN) = KBD_PRESSED then inc(smenu_cpos.y) else
        if KBD_get_arrow_key(ARROW_UP)   = KBD_PRESSED then dec(smenu_cpos.y);
	smenu_cpos.y := neg_mod(smenu_cpos.y, (SMENU_STAGES + 1) shr 1);

        { horizonal }
        if KBD_get_arrow_key(ARROW_RIGHT) = KBD_PRESSED then inc(smenu_cpos.x) else
        if KBD_get_arrow_key(ARROW_LEFT)  = KBD_PRESSED then dec(smenu_cpos.x);
        smenu_cpos.x := neg_mod(smenu_cpos.x, 2);

        if (smenu_cpos.x <> smenu_opos.x) or
	   (smenu_cpos.y <> smenu_opos.y) then
        begin
                smenu_compute_pcursor_pos;
                smenu_fill_oldcurpos := TRUE;
                SND_play(SND_BEEP2);
        end;
end;

procedure	smenu_draw;
begin
	{ draw backgroun d}
	if smenu_redraw_bg then
        begin
        	smenu_draw_bg;
                smenu_redraw_bg := FALSE;
        end;

        { draw planet }
        smenu_draw_planet;

        { draw menu }
        smenu_draw_menu;
end;

procedure	smenu_dispose;
begin
	{ nothing to do here }
end;

{ change }
procedure	smenu_change(param: pointer);
var	s	: boolean;
begin
	s := boolean(param);
        if s and (smenu_stage_target = smenu_stage_index) then
        begin
        	inc(smenu_stage_index);

                { save data }
                smenu_save_data;
        end;
        smenu_redraw_bg   := TRUE;
        smenu_redraw_menu := TRUE;
end;

function	smenu_setup_scene: SCENE;
var	s	: SCENE;
begin
	new(s);
        with s^ do
        begin
		init 	:= smenu_init;
        	update  := smenu_update;
        	draw    := smenu_draw;
        	dispose := smenu_dispose;
        	change  := smenu_change;
        	name    := SCENE_SMENU;
	end;
        smenu_setup_scene := s;
end;

procedure	smenu_init_assets;
begin
        smenu_bmp  := AST_get('smenu');
        smenu_font := AST_get('font');
end;

{ load saved data }
function	smenu_load_file: boolean;
var	f	: FILE;
begin
	if file_open(f, FALSE, F_SAVE_GAME) then
        begin
                file_read(smenu_stage_index, sizeof(uint8), f);
                file_close(f);
        	smenu_load_file := TRUE;
        end
	else	smenu_load_file := FALSE;
end;

procedure	smenu_clear_progress;
begin
        smenu_stage_index	:= 1;
        smenu_stage_target	:= 1;
        smenu_cpos.x		:= 0;
        smenu_cpos.y		:= 1;

        { remove save game file }
        file_remove(F_SAVE_GAME);
end;

{$endif}

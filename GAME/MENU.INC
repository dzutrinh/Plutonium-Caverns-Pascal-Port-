{*--------------------------------------------------------------------------*}
{*                              M E N U. I N C                              *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Menu component                                              *}
{* Prequisites: Core game engine                                            *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_MENU__}
{$define __CORE_MENU__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	MNU_NAME_LENGTH	= 14;
	MNU_MAX_BUTTON	= 8;

type    MENU_CALLBACK	= procedure;
	MENU = record
        	cpos	: int8;
                action	: uint8;
                count	: uint8;
                cbs	: array[0..MNU_MAX_BUTTON-1] of MENU_CALLBACK;
                text	: array[0..MNU_MAX_BUTTON-1] of string[MNU_NAME_LENGTH];
                width	: uint8;
                active	: boolean;
                redraw	: boolean;
	end;

{ MENU API }
procedure	MNU_init;                                                     forward;
procedure	MNU_create(var m: MENU; action: uint8);                       forward;
procedure	MNU_add_button(var m: MENU; text: string; cb: MENU_CALLBACK); forward;
procedure	MNU_draw(var m: MENU; dx, dy: int16);                         forward;
procedure	MNU_activate(var m: MENU; cpos: int16);                       forward;
procedure	MNU_update(var m: MENU; steps: int16);                        forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

var	MNU_font	: BITMAP;

procedure	MNU_init;
begin
        MNU_font := AST_get('font');
end;

procedure	MNU_create(var m: MENU; action: uint8);
begin
        m.count	 := 0;
        m.cpos	 := 0;
        m.active := FALSE;
        m.width	 := 0;
        m.redraw := TRUE;
        m.action := action;
end;

procedure	MNU_add_button(var m: MENU; text: string; cb: MENU_CALLBACK);
begin
	m.cbs [m.count] := cb;
        m.text[m.count] := #32+text;
        inc(m.count);
        if m.width < length(text) then m.width := length(text);
end;

procedure	MNU_draw(var m: MENU; dx, dy: int16);
const	XOFF	= 4;
	YOFF	= 6;
var	x, y,
	w, h, i	: int16;
begin
	if not m.active or not m.redraw then exit;

        w := (m.width + 2) * 8;
        h := (m.count + 1) * 12;

        x := dx - w shr 1;
        y := dy - h shr 1;

        VGA_fill_rect(x - 2, y - 2, w + 4, h + 4, 146);
        VGA_fill_rect(x - 1, y - 1, w + 2, h + 2, 255);
        VGA_fill_rect(x, y, w, h, 0);

        for i := 0 to m.count-1 do
        begin
                if m.cpos = i then
                        m.text[i, 1] := #6
                else	m.text[i, 1] := ' ';
        	VGA_draw_text_fast(MNU_font, m.text[i],
				   x + XOFF, y + YOFF + i * 12 + 3,
				   0, 0, FALSE);
        end;
        m.redraw := FALSE;
end;

procedure	MNU_activate(var m: MENU; cpos: int16);
begin
        m.active := TRUE;
        m.redraw := TRUE;
        if cpos < 0 then m.cpos := m.cpos else m.cpos := cpos;
end;

procedure	MNU_update(var m: MENU; steps: int16);
var	opos	: int8;
begin
	opos := m.cpos;
        if not m.active then exit;

        { ESC pressed ? }
        if KBD_get_button(3) = KBD_PRESSED then
        begin
                m.cbs[m.action];
                SND_play(SND_BEEP4);
                exit;
        end;

        { ENTER pressed ? }
        if KBD_get_button(2) = KBD_PRESSED then
        begin
                m.cbs[m.cpos];
                SND_play(SND_BEEP3);
        end;

        { cursor movement }
        if KBD_get_arrow_key(ARROW_DOWN) = KBD_PRESSED then inc(m.cpos) else
        if KBD_get_arrow_key(ARROW_UP) = KBD_PRESSED then dec(m.cpos);

        m.cpos := neg_mod(m.cpos, m.count);
        if m.cpos <> opos then
        begin
        	m.redraw := TRUE;
                SND_play(SND_BEEP2);
        end;
end;

{$endif}
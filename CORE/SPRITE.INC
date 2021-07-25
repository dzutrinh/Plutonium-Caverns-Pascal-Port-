{*--------------------------------------------------------------------------*}
{*                           S P R I T E . I N C                            *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Sprite management for the core game engine                  *}
{* Prequisites: TYPES.INC, GRAPHICS.INC                                     *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_SPRITE__}
{$define __CORE_SPRITE__}

{*--------------------------- I N T E R F A C E ----------------------------*}

type	SPRITE = record
		w, h,
                frame,
                row,
                count	: int16;
	end;

{ SPRITE API }
procedure	SPR_create(var s: SPRITE; w, h: int16);                             forward;
procedure	SPR_animate(var s: SPRITE; row, istart, iend,
			speed, steps: int16);			 	            forward;
procedure	SPR_draw_frame_skip(var s: SPRITE; bmp: BITMAP;
			frame, row, x, y, skip: int16; flip: boolean);              forward;
procedure	SPR_draw_skip(var s: SPRITE; bmp: BITMAP;
			x, y, skip: int16; flip: boolean);                          forward;
procedure	SPR_draw_frame(var s: SPRITE; bmp: BITMAP;
			frame, row, x, y: int16; flip: boolean);                    forward;
procedure	SPR_draw_frame_fast(var s: SPRITE; bmp: BITMAP;
			frame, row, x, y: int16);                                        forward;
procedure	SPR_draw(var s: SPRITE; bmp: BITMAP; x, y: int16; flip: boolean);   forward;
procedure	SPR_draw_fast(var s: SPRITE; bmp: BITMAP; x, y: int16);             forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

procedure	SPR_create(var s: SPRITE; w, h: int16);
begin
        s.w     := w;
        s.h     := h;
        s.frame := 0;
        s.count := 0;
        s.row   := 0;
end;

procedure	SPR_animate(var s: SPRITE; row, istart, iend, speed, steps: int16);
begin
	{ nothing to animate }
        if istart = iend then
        begin
        	s.count := 0;
                s.frame := istart;
                s.row   := row;
                exit;
        end;

        { row changed }
        if s.row <> row then
        begin
        	s.count := 0;
                {if iend > istart then
                	s.frame := istart
                else	s.frame := iend;}
                s.frame := mini(istart, iend);
                s.row   := row;
        end;

        if (istart < iend) and (s.frame < istart) then
        	s.frame := istart
        else
        if (iend < istart) and (s.frame < iend) then
        	s.frame := iend;

        { animate }
        inc(s.count, steps);
        if (s.count > speed) then
        begin
        	if istart < iend then
                begin
                        inc(s.frame);
                        if s.frame > iend then s.frame := istart;
                end
                else
                begin
                	dec(s.frame);
                        if s.frame < iend then	s.frame := istart;
                end;
                dec(s.count, speed);
        end;
end;

procedure	SPR_draw_frame_skip(var s: SPRITE; bmp: BITMAP;
			frame, row, x, y, skip: int16; flip: boolean);
begin
        VGA_draw_bitmap_region_skip(bmp, s.w * frame, s.h * row,
		s.w, s.h, x, y, skip, flip);
end;

procedure	SPR_draw_skip(var s: SPRITE; bmp: BITMAP;
			x, y, skip: int16; flip: boolean);
begin
	SPR_draw_frame_skip(s, bmp, s.frame, s.row, x, y, skip, flip);
end;

procedure	SPR_draw_frame(var s: SPRITE; bmp: BITMAP;
			frame, row, x, y: int16; flip: boolean);
begin
	VGA_draw_bitmap_region(bmp, s.w * frame, s.h * row,
		s.w, s.h, x, y, flip);
end;

procedure	SPR_draw(var s: SPRITE; bmp: BITMAP;
			 x, y: int16; flip: boolean);
begin
	SPR_draw_frame(s, bmp, s.frame, s.row, x, y, flip);
end;

procedure	SPR_draw_frame_fast(var s: SPRITE; bmp: BITMAP;
			frame, row, x, y: int16);
begin
        VGA_draw_bitmap_region_fast(bmp, s.w * frame, s.h * row,
		s.w, s.h, x, y);
end;

procedure	SPR_draw_fast(var s: SPRITE; bmp: BITMAP; x, y: int16);
begin
        SPR_draw_frame_fast(s, bmp, s.frame, s.row, x, y);
end;

{$endif}
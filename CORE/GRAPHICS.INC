{*--------------------------------------------------------------------------*}
{*                         G R A P H I C S . I N C                          *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : VGA graphics library for the core game engine               *}
{* Prequisites: TYPES.INC                                                   *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __CORE_GRAPHICS__}
{$define __CORE_GRAPHICS__}

{*--------------------------- I N T E R F A C E ----------------------------*}

const	VGA_WIDTH	= 320;
	VGA_HEIGHT	= 200;
        VGA_SIZE	= VGA_WIDTH * VGA_HEIGHT;

type	VGA_ARRAY  = array[0..VGA_SIZE-1] of uint8;
	RASTER = ^VGA_ARRAY;

	BITMAP = ^BITMAP_TYPE;
	BITMAP_TYPE = record
                w, h	: uint16;
                data	: RASTER;
        end;

{ VGA GRAPHICS API }
function	VGA_init_graphics: boolean;                                           forward;
procedure	VGA_destroy_graphics;                                                 forward;
procedure       VGA_draw_frame;                                                       forward;
procedure	VGA_set_palette;                                                      forward;
function	VGA_clip(var sx, sy, sw, sh, dx, dy: int16; flip: boolean): boolean;  forward;
function	VGA_clip_rect(var x, y, w, h: int16): boolean;                        forward;
function	VGA_create_bitmap(w, h: uint16): BITMAP;                              forward;
procedure	VGA_destroy_bitmap(bmp: BITMAP);                                      forward;
function        VGA_get_dark_value(color, count: uint8): uint8;                       forward;
procedure	VGA_set_palette_darkness(d: uint8);                                   forward;
procedure	VGA_clear(color: uint8);                                              forward;
procedure	VGA_set_viewport(x, y, w, h: int16);                                  forward;
procedure	VGA_reset_viewport;                                                   forward;
procedure	VGA_clear_viewport(color: uint8);                                     forward;
function	VGA_load_bitmap(name: string): BITMAP;                                forward;
function	VGA_save_bitmap(bmp: BITMAP; name: string): boolean;                  forward;
procedure	VGA_fill_rect(x, y, w, h: int16; color: uint8);                       forward;
procedure	VGA_draw_rect(dx, dy, w, h: int16; color: uint8);                     forward;
procedure       VGA_draw_bitmap_region_fast(bmp: BITMAP;
			sx, sy, sw, sh, dx, dy: int16);                               forward;
procedure	VGA_draw_bitmap_region(bmp: BITMAP; sx, sy, sw, sh,
			dx, dy: int16; flip: boolean);                                forward;
procedure	VGA_draw_bitmap_fast(bmp: BITMAP; dx, dy: int16);                     forward;
procedure	VGA_draw_bitmap(bmp: BITMAP; x, y: int16; flip: boolean);             forward;
procedure       VGA_draw_substr_fast(f: BITMAP; t: string; dx, dy, hs, vs: int16;
			istart, iend: uint16; cent: boolean);                         forward;
procedure	VGA_draw_text_fast(f: BITMAP; t: string; dx, dy,
			hs, vs: int16; cent: boolean);                                forward;
procedure	VGA_wait_retrace;                                                     forward;
procedure	VGA_toggle_clipping(flag: boolean);                                   forward;
procedure	VGA_direct_video(flag: boolean);                                      forward;

{*---------------------- I M P L E M E N T A T I O N -----------------------*}

var	VGA_frame	: RASTER;	{ double-buffering              }
        VGA_current	: RASTER;	{ current working frame buffer  }
	VGA_memory	: RASTER;	{ pointer to VGA memory         }
        VGA_viewport	: RECT;		{ current viewport for clipping }
        VGA_clipping	: boolean;	{ clipping flag                 }
        VGA_alpha	: uint8;	{ transparent color             }
	VGA_direct	: boolean;	{ draw directly to VGA memory   }

        { for faster offset calculations }
	VGA_yoffset	: array[0..VGA_HEIGHT-1] of uint16;

procedure	VGA_direct_video(flag: boolean);
begin
	VGA_direct := flag;
        if flag then
		VGA_current := VGA_memory	{ direct VGA write }
        else	VGA_current := VGA_frame;	{ double buffering }
end;

function	VGA_init_graphics: boolean;
var	i	: uint16;
begin
	VGA_init_graphics := FALSE;

        { allocate memory for the double buffer }
        VGA_frame := RASTER(MEM_malloc(VGA_WIDTH * VGA_HEIGHT));
        if VGA_frame = nil then
	begin
		ERR_throw_no_param('Memory allocation error!');
                exit;
        end;

        { precompute the y lookup table }
        for i := 0 to VGA_HEIGHT-1 do VGA_yoffset[i] := i * VGA_WIDTH;

        { set defaults }
	VGA_alpha := 170;
        VGA_direct_video(FALSE);
        VGA_toggle_clipping(TRUE);
        VGA_set_viewport(0, 0, VGA_WIDTH, VGA_HEIGHT);
        VGA_clear(0);

        { obtain the pointer to direct VGA memory }
{$ifdef	VER70}                          { Real mode with segment, offset   }
        VGA_memory := ptr($A000, 0);
{$else}					{ Protected mode flat address      }
	VGA_memory := pointer($A0000);
{$endif}

        asm
        	mov	ax, 0013h       { switch to mode 13h }
                int	10h
        end;

        VGA_set_palette;		{ apply default game palette }

        VGA_init_graphics := TRUE;
end;

procedure	VGA_destroy_graphics;
begin
        MEM_free(VGA_frame);		{ free double buffer's memory }
	asm				{ back to text mode           }
        	mov	ax, 0003h
                int	10h
        end;
end;

function	VGA_coord(x, y: int16): uint16;
begin
	VGA_coord := VGA_yoffset[y] + x;
end;

procedure       VGA_draw_frame;
begin
        MEM_copy(VGA_memory^, VGA_frame^, VGA_SIZE);
end;

procedure	VGA_set_palette;
var	i	: int16;
begin
        port[$3C8] := 0;
	for i := 0 to 255 do
        begin
                port[$3C9] := GAME_CMAP[i*3+0];
                port[$3C9] := GAME_CMAP[i*3+1];
                port[$3C9] := GAME_CMAP[i*3+2];
        end;
end;

function        VGA_get_dark_value(color, count: uint8): uint8;
var	r, g, b	: uint8;
	i	: int16;
begin
        r := uint8(color shr 5);
        g := uint8(color shl 3);
        g := uint8(g shr 5);
        b := uint8(color shl 6);
        b := b shr 6;

        for i := 0 to count-1 do
        begin
                if r > 0 then dec(r);
                if g > 0 then dec(g);
                if odd(i) then if b > 0 then dec(b);
        end;
        r := r shl 5;
        g := g shl 2;
        VGA_get_dark_value := r or g or b;
end;

procedure	VGA_set_palette_darkness(d: uint8);
var	i, j	: int16;
begin
        i := 0;
        if d = 0 then
        begin
                VGA_set_palette;
                exit;
        end;

        port[$3C8] := 0;
        for i := 0 to 255 do
        begin
        	j := VGA_get_dark_value(i, d);
		port[$3C9] := GAME_CMAP[j*3];
                port[$3C9] := GAME_CMAP[j*3+1];
                port[$3C9] := GAME_CMAP[j*3+2];
        end;
end;

procedure	VGA_clear(color: uint8);
begin
        MEM_set(VGA_current^, color, VGA_SIZE);
end;

procedure	VGA_set_viewport(x, y, w, h: int16);
begin
        VGA_viewport.x := x;
        VGA_viewport.y := y;
        VGA_viewport.w := w;
        VGA_viewport.h := h;
end;

procedure	VGA_reset_viewport;
begin
        VGA_viewport.x := 0;
        VGA_viewport.y := 0;
        VGA_viewport.w := VGA_WIDTH;
        VGA_viewport.h := VGA_HEIGHT;
end;

procedure	VGA_clear_viewport(color: uint8);
begin
        VGA_fill_rect(VGA_viewport.x, VGA_viewport.y, VGA_viewport.w, VGA_viewport.h, color);
end;

function	VGA_clip(var sx, sy, sw, sh, dx, dy: int16; flip: boolean): boolean;
var	ow, oh	: int16;
begin
	ow := sw;
        if dx < VGA_viewport.x then
        begin
                dec(sw, VGA_viewport.x - dx);
                if not flip then inc(sx, ow-sw);
                dx := VGA_viewport.x;
        end;
        if dx + sw >= VGA_viewport.x + VGA_viewport.w then
        begin
                dec(sw, (dx+sw) - (VGA_viewport.x + VGA_viewport.w));
                if flip then inc(sx, ow-sw);
        end;
        oh := sh;
        if dy < VGA_viewport.y then
        begin
                dec(sh, VGA_viewport.y - dy);
                inc(sy, oh - sh);
                dy := VGA_viewport.y;
        end;
        if dy + sh >= VGA_viewport.y + VGA_viewport.h then
        	dec(sh, (dy+sh)-(VGA_viewport.y + VGA_viewport.h));
        VGA_clip := (sw > 0) and (sh > 0);
end;

function	VGA_clip_rect(var x, y, w, h: int16): boolean;
begin
	{ left }
        if x < VGA_viewport.x then
	begin
                dec(w, VGA_viewport.x - x);
                x := VGA_viewport.x;
        end;

        { right }
        if x + w >= VGA_viewport.x + VGA_viewport.w then
        	dec(w, (x + w) - (VGA_viewport.x + VGA_viewport.w));

        { top }
        if y < VGA_viewport.y then
        begin
        	dec(h, VGA_viewport.y - y);
                y := VGA_viewport.y;
        end;

        { bottom }
        if y + h >= VGA_viewport.y + VGA_viewport.h then
        	dec(h, (y + h) - (VGA_viewport.y + VGA_viewport.h));

        VGA_clip_rect := (w > 0) and (h > 0);
end;

function	VGA_create_bitmap(w, h: uint16): BITMAP;
var	bmp	: BITMAP;
begin
	VGA_create_bitmap := nil;
        if memavail < w * h then exit;
        new(bmp);
        bmp^.w    := w;
        bmp^.h    := h;
        bmp^.data := RASTER(MEM_malloc(w * h));
        VGA_create_bitmap := bmp;
end;

procedure	VGA_destroy_bitmap(bmp: BITMAP);
begin
        if bmp = nil then exit;
        if bmp^.data <> nil then MEM_free(bmp^.data);
	dispose(bmp);
end;

function	VGA_load_bitmap(name: string): BITMAP;
var	w, h,
	i, nr	: uint16;
	bmp	: BITMAP;
        f	: FILE;
begin
	VGA_load_bitmap := nil;
        if not file_open(f, FALSE, name) then exit;
        file_read(w, 2, f);
        file_read(h, 2, f);
        if (w < 0) or (h < 0) or (w > 4096) or (h > 4096) then
        begin
        	file_close(f);
                exit;
        end;

        bmp := VGA_create_bitmap(w, h);
        if bmp = nil then
        begin
        	file_close(f);
                exit;
        end;
        for i := 0 to h-1 do
        begin
	        nr := file_read(bmp^.data^[i*bmp^.w], bmp^.w, f);
                if nr <> bmp^.w then
                begin
                	file_close(f);
                        VGA_destroy_bitmap(bmp);
                        exit;
                end
        end;
        file_close(f);
        VGA_load_bitmap := bmp;
end;

function	VGA_save_bitmap(bmp: BITMAP; name: string): boolean;
var	f	: FILE;
	nw, i	: uint16;
begin
        VGA_save_bitmap := FALSE;
        if bmp = nil then exit;
        if not file_open(f, TRUE, name) then exit;

        file_write(bmp^.w, 2, f);
        file_write(bmp^.h, 2, f);
        for i := 0 to bmp^.h-1 do
        begin
	        nw := file_write(bmp^.data^[i*bmp^.w], bmp^.w, f);
                if nw <> bmp^.w then
                begin
                	file_close(f);
                        VGA_destroy_bitmap(bmp);
                        exit;
                end
        end;
        file_close(f);
	VGA_save_bitmap := TRUE;
end;

procedure	VGA_fill_rect(x, y, w, h: int16; color: uint8);
var	o	: uint16;
begin
	if VGA_clipping and not VGA_clip_rect(x, y, w, h) then exit;

        o := VGA_coord(x, y);
	while h > 0 do
        begin
                MEM_set(VGA_current^[o], color, w);
        	inc(o, VGA_WIDTH);
                dec(h);
        end;
end;

procedure	VGA_draw_rect(dx, dy, w, h: int16; color: uint8);
var	ox, oy,
	y	: int16;
	offs	: uint16;
begin
        ox := dx;
        oy := dy;
	if VGA_clipping and not VGA_clip_rect(dx, dy, w, h) then exit;
	if dy = oy then
        begin
                offs := VGA_coord(dx, dy);
                MEM_set(VGA_current^[offs], color, w);
        end;
        if (dy + h >= 0) then
        begin
        	offs := VGA_coord(dx, dy+h-1);
                MEM_set(VGA_current^[offs], color, w);
        end;
        if (dx = ox) then
        begin
        	offs := VGA_coord(dx, dy+1);
        	for y := dy+1 to dy+h-2 do
                begin
                        VGA_current^[offs] := color;
                        inc(offs, VGA_WIDTH);
                end;
        end;
        if (dx+w >= 0) then
        begin
        	offs := VGA_coord(dx+w-1, dy+1);
        	for y := dy+1 to dy+h-2 do
                begin
                        VGA_current^[offs] := color;
                        inc(offs, VGA_WIDTH);
                end;
        end;
end;

procedure       VGA_draw_bitmap_region_fast(bmp: BITMAP; sx, sy,
			sw, sh, dx, dy: int16);
var	y	: int16;
	offs,
	boff	: uint16;
begin
        if bmp = nil then exit;

	if VGA_clipping and not
	   VGA_clip(sx, sy, sw, sh, dx, dy, FALSE) then exit;

        offs := VGA_coord(dx, dy);
        boff := mini(uint16(bmp^.w * sy + sx), bmp^.w * bmp^.h-1);
        for y := dy to dy+sh-1 do
        begin
                MEM_copy(VGA_current^[offs], bmp^.data^[boff], sw);
                inc(offs, VGA_WIDTH);
                inc(boff, bmp^.w);
        end;
end;

{ fast blit a bitmap at (dx, dy), ignoring alpha, clipped }
procedure	VGA_draw_bitmap_fast(bmp: BITMAP; dx, dy: int16);
begin
	VGA_draw_bitmap_region_fast(bmp, 0, 0, bmp^.w, bmp^.h, dx, dy);
end;

{ blit a sub-string from text at (dx, dy) using (hs, vs) as horizonal gap
  and vertical gap using bitmap font, ignoring alpha, clipped }
procedure       VGA_draw_substr_fast(f: BITMAP; t: string;
				     dx, dy, hs, vs: int16; istart, iend: uint16;
				     cent: boolean);
var	len, cw, ch, i	: uint16;
	c		: uint8;
	x, y, sx, sy	: int16;
begin
	len  := length(t);
        x    := dx;
        y    := dy;
        cw   := f^.w div 16;
        ch   := cw;
        iend := mini(len, iend);

        if cent then
        begin
        	dec(dx, (cw+hs)*iend shr 1);
                x := dx;
        end;

        for i := 0 to iend-1 do
        begin
        	c := uint8(t[i+1]);

                { new line }
                if c = 13 then
                begin
                	x := dx;
                        inc(y, ch + vs);
                        continue;
                end;

                if i >= istart then
                begin
                	sx := c mod 16;
                        sy := c div 16;

                        { draw char }
                        VGA_draw_bitmap_region_fast(f, sx * cw, sy * ch,
				cw, ch, x, y);
                end;
                inc(x, cw + hs);
        end;
end;

{ blit a text using bitmap font, ignoring alpha, clipped }
procedure	VGA_draw_text_fast(f: BITMAP; t: string;
				dx, dy, hs, vs: int16; cent: boolean);
begin
        VGA_draw_substr_fast(f, t, dx, dy, hs, vs, 0, length(t), cent);
end;

procedure	VGA_draw_bitmap_region_skip(	bmp: BITMAP;
						sx, sy, sw, sh, dx, dy: int16;
						skip: int16; flip: boolean);
var	x, y, dir	: int16;
	offs, boff	: uint16;
	pixel		: uint8;
begin
	if bmp = nil then exit;

	if flip then dir := -1 else dir := 1;

        if VGA_clipping and not VGA_clip(sx, sy, sw, sh, dx, dy, flip) then exit;

        offs := VGA_coord(dx, dy);
        boff := mini(uint16(bmp^.w * sy + sx), bmp^.w * bmp^.h-1);

        if flip then inc(boff, sw-1);

        for y := 0 to sh-1 do
        begin
        	for x := 0 to sw-1 do
                begin
                	pixel := bmp^.data^[boff];

                        { check for alpha color and not skip }
                        if (pixel <> VGA_alpha) and
                           ((skip = 0) or
			    ((x mod skip <> 0) and
			     (y mod skip <> 0))) then
                                VGA_current^[offs] := pixel;
                        inc(boff, dir);
                        inc(offs);
                end;
                inc(boff, bmp^.w - sw * dir);
                inc(offs, VGA_WIDTH - sw);
        end;
end;

procedure	VGA_draw_bitmap_region(bmp: BITMAP;
					sx, sy, sw, sh, dx, dy: int16;
					flip: boolean);
begin
        VGA_draw_bitmap_region_skip(bmp, sx, sy, sw, sh, dx, dy, 0, flip);
end;

procedure	VGA_draw_bitmap(bmp: BITMAP; x, y: int16; flip: boolean);
begin
	VGA_draw_bitmap_region(bmp, 0, 0, bmp^.w, bmp^.h, x, y, flip);
end;

procedure	VGA_wait_retrace;
begin
	while (port[$3DA] and 8) <> 0 do;
	while (port[$3DA] and 8) =  0 do;
end;

procedure	VGA_toggle_clipping(flag: boolean);
begin
	VGA_clipping := flag;
end;

{$endif}
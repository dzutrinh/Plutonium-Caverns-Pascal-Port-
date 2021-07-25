uses	crt;

{$i ..\core\types.inc}
{$i ..\core\error.inc}
{$i ..\core\utils.inc}
{$i ..\core\graphics.inc}

var	filename	: string;
        bmp		: BITMAP;
begin
	filename := paramstr(1);
        if filename = '' then
		ERR_die('usage: ', 'viewer image.bin');

	bmp := VGA_load_bitmap(filename);
        if bmp <> nil then
        begin
                VGA_init_graphics;
                VGA_direct_video(TRUE);
		VGA_draw_bitmap_fast(bmp,
			160 - bmp^.w div 2, 100 - bmp^.h div 2);
                readkey;
                VGA_destroy_graphics;
        end;
end.

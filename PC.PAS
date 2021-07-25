{*--------------------------------------------------------------------------*}
{*                                P C . P A S                               *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{*  Plutonium Caverns for DOS                                               *}
{*  by Jani Nyk„nen, 2019                                                   *}
{*  Pascal port by Trinh D.D. Nguyen, 2021                                  *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{*  Supported Pascal compilers:                                             *}
{*  . TMT Pascal 3.9           (32-bit)                                     *}
{*  . Borland/Turbo Pascal 7.0 (16-bit)                                     *}
{*--------------------------------------------------------------------------*}
{*  Compiling:                                                              *}
{*  . Borland/Turbo Pascal: TPC or BPC PC.PAS                               *}
{*  . TMT Pascal          : TMTPC PC.PAS                                    *}
{*  Or simply type 'MAKE' to build using the included MAKEFILE, default     *}
{*  compiler set in the MAKEFILE is TMT.                                    *}
{*--------------------------------------------------------------------------*}
{$i preps.inc        }

uses	crt, dos;

{*--------------------------- C O R E  E N G I N E -------------------------*}
{$i core\types.inc   }
{$i core\error.inc   }
{$i core\utils.inc   }
{$i core\input.inc   }
{$i core\sound.inc   }
{$i core\graphics.inc}
{$i core\sprite.inc  }
{$i core\tilemap.inc }
{$i core\asset.inc   }
{$i core\trans.inc   }
{$i core\app.inc     }

{*----------------------------- T H E  G A M E -----------------------------*}
{$i game\globals.inc }
{$i game\menu.inc    }
{$i game\intro.inc   }
{$i game\smenu.inc   }
{$i game\game.inc    }
{$i game\title.inc   }
{$i game\story.inc   }

begin
	if not APP_init then
        begin
        	writeln('ERROR: ', ERR_get);
                exit;
        end;

        APP_add_scene(game_setup_scene,  FALSE);
        APP_add_scene(smenu_setup_scene, FALSE);
        APP_add_scene(title_setup_scene, FALSE);
        APP_add_scene(story_setup_scene, FALSE);
        APP_add_scene(intro_setup_scene, TRUE );

        KBD_add_button(0, KEY_Z    );	{ currently unused     }
        KBD_add_button(1, KEY_R    );	{ for restarting level }
        KBD_add_button(2, KEY_ENTER);
        KBD_add_button(3, KEY_ESC  );

        APP_run;
end.

#include <kicker.h>
#include <malloc.h>
#include <loader.h>
#include <memory.h>
#include <printf.h>
#include <string.h>


struct Boot_Module *
save_modules_info (multiboot_info_t *mbi, struct Kicker_Info *info)
{
  void *mem;
  int i;
  module_t *from;
  struct Boot_Module *mod;
  
  printf ("Saving %d servers at 0x%x\n", mbi->mods_count, mbi->mods_addr);
  
  mem = malloc (mbi->mods_count * sizeof (struct Boot_Module));
  info->modules_count = mbi->mods_count;
  info->modules_address = (uint32_t) mem;
  
  for (i = 0,
	 from = (module_t *) mbi->mods_addr,
	 mod = (struct Boot_Module *) mem;
      i < (int) mbi->mods_count;
      i++, from++, mod++)
    {
      int len;

      printf ("  Module `%s'\n", (char *) from->string);
      
      mod->start = from->mod_start;
      mod->end = from->mod_end;
      
      len = strlen ((char *) from->string);
      mod->string = (char *) malloc (len + 1);
      memcpy (mod->string, (char *) from->string, len + 1);
    }

  return (struct Boot_Module *) mem;
}

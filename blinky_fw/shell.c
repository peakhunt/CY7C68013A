#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "usb_cdc.h"
#include "shell.h"

#define SHELL_MAX_LINE_LEN        256
#define SHELL_MAX_ARGS            4

typedef void (*shell_command_handler)(uint8_t argc);

typedef struct
{
  const char*           command;
  const char*           description;
  shell_command_handler handler;
} ShellCommand;

static void shell_command_help(uint8_t argc);
static void shell_command_version(uint8_t argc);
static void shell_command_uptime(uint8_t argc);


__xdata static uint16_t cmd_buffer_ndx  = 0;
__xdata static uint8_t  cmd_buffer[SHELL_MAX_LINE_LEN];
__xdata static uint8_t  print_buffer[SHELL_MAX_LINE_LEN];
static const char* cmd_args[SHELL_MAX_ARGS];

__xdata static ShellCommand _commands[] =
{
  {
    "help",
    "show this command",
    shell_command_help,
  },
  {
    "version",
    "show version",
    shell_command_version,
  },
  {
    "uptime",
    "show system uptime",
    shell_command_uptime,
  },
};

static void
shell_prompt(void)
{
  sprintf(print_buffer, "\r\nMBS> ");
  cdc_print_string(print_buffer);
}

static void
shell_printf(const char* fmt, ...)
{
  va_list   args;
  int       len;

  va_start(args, fmt);
  len = vsprintf(print_buffer, fmt, args);
  va_end(args);
  cdc_print_string(print_buffer);
}

static void
shell_command_help(uint8_t argc)
{
  size_t i;

  (void)argc;

  shell_printf("\r\n");

  for(i = 0; i < sizeof(_commands)/sizeof(ShellCommand); i++)
  {
    shell_printf("%-20s: ", _commands[i].command);
    shell_printf("%s\r\n", _commands[i].description);
  }
}

static void
shell_command_version(uint8_t argc)
{
  (void)argc;

  shell_printf("\r\n");
  shell_printf("0.1\r\n");
}

static void
shell_command_uptime(uint8_t argc)
{
  uint32_t uptime = get_uptime();
  (void)argc;

  shell_printf("\r\n");
  shell_printf("System Uptime: %lu sec\r\n", uptime);
}

static void
shell_execute_command(char* cmd)
{
  uint8_t               argc = 0;
  uint8_t               i;
  char                  *s;

  while((s = strtok(argc  == 0 ? cmd : NULL, " \t")) != NULL)
  {
    if(argc >= SHELL_MAX_ARGS)
    {
      shell_printf("\r\nError: too many arguments\r\n");
      return;
    }
    cmd_args[argc++] = s;
  }

  if(argc == 0)
  {
    return;
  }

  for(i = 0; i < sizeof(_commands)/sizeof(ShellCommand); i++)
  {
    if(strcmp(_commands[i].command, cmd_args[0]) == 0)
    {
      shell_printf("\r\nExecuting %s\r\n", cmd_args[0]);
      _commands[i].handler(argc);
      return;
    }
  }

  shell_printf("%s", "\r\nUnknown Command: ");
  shell_printf("%s", cmd_args[0]);
  shell_printf("%s", "\r\n");
}

void
shell_handle_rx(uint8_t b)
{
  if(b != '\r' && cmd_buffer_ndx < SHELL_MAX_LINE_LEN)
  {
    if(b == 0x08 || b == 0x7f)
    {
      if(cmd_buffer_ndx > 0)
      {
        shell_printf("%c%c%c", 0x08, 0x20, 0x08);
        cmd_buffer_ndx--;
      }
    }
    else
    {
      shell_printf("%c", b);
      cmd_buffer[cmd_buffer_ndx++] = b;
    }
  }
  else if(b == '\r')
  {
    cmd_buffer[cmd_buffer_ndx++] = '\0';

    shell_execute_command((char*)cmd_buffer);

    cmd_buffer_ndx = 0;
    shell_prompt();
  }
  else if(cmd_buffer_ndx >= SHELL_MAX_LINE_LEN)
  {
    cmd_buffer_ndx = 0;
  }
}

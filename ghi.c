
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h> // usleep
#include <sys/wait.h> // WEXITSTATUS
#include "commander/commander.h"
#include "strdup/strdup.h"
#include "tempdir/tempdir.h"
#include "asprintf/asprintf.h"
#include "http-get/http-get.h"
#include "parse-repo/parse-repo.h"
#include "mkdirp/mkdirp.h"
#include "logger/logger.h"
#include "batch/batch.h"

#define TARBALL_FORMAT "https://github.com/%s/%s/archive/%s.tar.gz"

#define ERROR(...) logger_error("error", __VA_ARGS__);

struct options {
  char *dir;
  int show_output;
  char *command;
};


/**
 * Options.
 */

struct options opts;

/**
 * Set the ouput directory.
 */

void
set_output_directory(command_t *self) {
  if (opts.dir) free(opts.dir);
  opts.dir = strdup((char *) self->arg);
}

/**
 * Set the install command.
 */

void
set_install_command(command_t *self) {
  opts.command = (char *) self->arg;
}

/**
 * Show output.
 */

void
set_show_output(command_t *self) {
  opts.show_output = 1;
}

/**
 * Spawn the given `command`, optionally piping
 * output to STDOUT.
 */

int
spawn_command(const char *command, int pipe) {
  FILE *fp;
  int rc;

  fp = popen(command, "r");
  if (NULL == fp) {
    ERROR("failed to pipe output");
    return 1;
  }

  char buf[256];
  while (0 != fgets(buf, sizeof(buf), fp)) {
    if (pipe) printf("%s", buf);
  }

  rc = pclose(fp);
  if (-1 == rc) {
    ERROR("unable to close pipe");
    return 1;
  }

  return WEXITSTATUS(rc);
}

/**
 * Install the given `repo`.
 */

int
install(char *repo) {
  int rc = 0;
  char *owner = NULL;
  char *name = NULL;
  char *version = NULL;
  char *tarball = NULL;
  char *file = NULL;
  char *untar_command = NULL;
  char *install_command = NULL;

  // parse repo
  if (!(owner = parse_repo_owner(repo, NULL))) {
    ERROR("unable to parse repository owner ('%s')", repo);
    rc = 1;
    goto done;
  }

  if (!(name = parse_repo_name(repo))) {
    ERROR("unable to parse repository name ('%s')", repo);
    rc = 1;
    goto done;
  }

  if (!(version = parse_repo_version(repo, "master"))) {
    ERROR("unable to parse repository version ('%s')", repo);
    rc = 1;
    goto done;
  }

  #define ASPRINTF(...) ({                                           \
    rc = asprintf(__VA_ARGS__);                                      \
    if (-1 == rc) {                                                  \
      ERROR("unable to allocate memory");                            \
      rc = 1;                                                        \
      goto done;                                                     \
    }                                                                \
  });

  ASPRINTF(&tarball, TARBALL_FORMAT, owner, name, version);
  ASPRINTF(&file, "%s/%s-%s.tar.gz", opts.dir, name, owner);
  ASPRINTF(
      &untar_command
    , "cd %s && tar -xf %s-%s.tar.gz"
    , opts.dir
    , name
    , owner
  );
  ASPRINTF(
      &install_command
    , "cd %s/%s-%s && %s"
    , opts.dir
    , name
    , version
    , opts.command
  );

  #undef ASPRINTF

  logger_info("fetch", tarball);
  rc = http_get_file(tarball, file);
  if (-1 == rc) {
    ERROR("failed to fetch %s", tarball);
    rc = 1;
    goto done;
  }

  logger_info("untar", file);
  if (0 != spawn_command(untar_command, opts.show_output)) {
    ERROR("unable to untar %s", file);
    rc = 1;
    goto done;
  }

  if (0 != spawn_command(install_command, opts.show_output)) {
    ERROR("unable to install %s/%s-%s", opts.dir, name, version);
    rc = 1;
    goto done;
  }


done:
  free(owner);
  free(version);
  free(name);
  free(tarball);
  free(file);
  free(untar_command);
  free(install_command);

  return rc;
}


void *
th_install(void *data) {
  char *repo = (char *) data;
  int rc = install(repo);
  // TODO signal failure
  if (0 != rc) {
    ERROR("failed to install %s", repo);
  } else {
    logger_info("installed", repo);
  }
  return NULL;
}

batch_error_t
install_repos(char **repos, int count) {
  batch_t *batch = NULL;
  batch_error_t error;

  batch = batch_new(5);
  if (NULL == batch) {
    ERROR("unable to allocate memory");
    return BATCH_MALLOC_ERROR;
  }

  for (int i = 0; i < count; ++i) {
    error = batch_push(batch, th_install, (void *) repos[i]);
    if (0 != error) {
      batch_end(batch);
      return error;
    }
    usleep(500);
  }

  error = batch_wait(batch);
  if (0 != error) {
    batch_end(batch);
    return error;
  }

  return batch_end(batch);
}

/**
 * Entry point.
 */

int
main(int argc, char **argv) {
  command_t program;
  int rc = 0;
  char *temp = NULL;
  batch_error_t batch_error;

  temp = gettempdir();
  if (NULL == temp) {
    ERROR("unable to allocate memory (gettempdir())");
    return 1;
  }
  opts.dir = temp;
  opts.show_output = 0;
  opts.command = "make install";

  command_init(&program, "ghi", GHI_VERSION);
  program.usage = "[options] <repo ...>";
  command_option(
      &program
    , "-o"
    , "--out <dir>"
    , "set the download directory"
    , set_output_directory
  );
  command_option(
      &program
    , "-s"
    , "--show-output"
    , "show install output"
    , set_show_output
  );
  command_option(
      &program
    , "-c"
    , "--command <command>"
    , "use a custom command"
    , set_install_command
  );
  command_parse(&program, argc, argv);

  if (0 == program.argc) {
    ERROR("at least one repository is required");
    rc = 1;
    goto done;
  }

  rc = mkdirp(opts.dir, 0777);
  if (-1 == rc) {
    ERROR("unable to create directory %s", opts.dir);
    rc = 1;
    goto done;
  }

  batch_error = install_repos(program.argv, program.argc);
  if (0 != batch_error) {
    ERROR(batch_error_string(batch_error));
    rc = 1;
  }

done:
  command_free(&program);
  free(opts.dir);
  return rc;
}

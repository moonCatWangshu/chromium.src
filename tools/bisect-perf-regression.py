Example usage using SVN revisions:
Be aware that if you're using the git workflow and specify an SVN revision,
the script will attempt to find the git SHA1 where SVN changes up to that
Example usage using git hashes:
from auto_bisect import request_build
# Possible return values from BisectPerformanceMetrics.RunTest.
# Maximum time in seconds to wait after posting build request to the try server.
# the try server.
# dependency repositories. This patch send along with DEPS patch to try server.
# When a build requested is posted with a patch, bisect builders on try server,
Estimated Confidence: %(confidence).02f%%"""
To reproduce on a performance try bot:
4. Send your try job to the try server. \
REPRO_STEPS_TRYJOB_TELEMETRY = """
To reproduce on a performance try bot:
%(command)s
(Where <bot-name> comes from tools/perf/run_benchmark --browser=list)

For more details please visit
https://sites.google.com/a/chromium.org/dev/developers/performance-try-bots
"""

      # Build archive for x64 is still stored with the "win32" suffix.
      # See chromium_utils.PlatformName().
      # Android builds are also archived with the "full-build-linux prefix.
  """Returns the URL to download the build from."""
    """Returns the Google Cloud Storage root folder name."""
    Downloaded file path if exists, otherwise None.
# This is copied from build/scripts/common/chromium_utils.py.
# This was copied from build/scripts/common/chromium_utils.py.
  # the python module does, so we have to fall back to the python zip module
  # on Mac if the file size is greater than 4GB.
  """Writes text to a file, raising an RuntimeError on failure."""
  """Writes text to a file, raising an RuntimeError on failure."""
  """Formats file paths in the given patch text to Unix-style paths."""
  if not diff_text:
    return None
  diff_lines = diff_text.split('\n')
  for i in range(len(diff_lines)):
    line = diff_lines[i]
    if line.startswith('--- ') or line.startswith('+++ '):
      diff_lines[i] = line.replace('\\', '/')
  return '\n'.join(diff_lines)
  """Parses the vars section of the DEPS file using regular expressions.
    A dictionary in the format {depot: revision} if successful, otherwise None.
  """Waits until build is produced by bisect builder on try server.
    bot_name: Builder bot name on try server.
    builder_host Try server host name.
    builder_port: Try server port.
    build_request_id: A unique ID of the build request posted to try server.
  # Build number on the try server.
  # Interval to check build status on try server in seconds.
    # To avoid overloading try server with status check requests, we check
    # build status for every 10 minutes.
        # Get the build number on try server for the current build.
        build_num = request_build.GetBuildNumFromBuilder(
      # on the the try server.
      build_status, status_link = request_build.GetBuildStatus(
      if build_status == request_build.FAILED:
  """Adds new revisions to the revision_data dictionary and initializes them.
    revision_data: A dictionary to add the new revisions into.
        Existing revisions will have their sort keys adjusted.
    revision_data_sorted: Sorted list of (revision, revision data) pairs.
    commit_position = self.source_control.GetCommitPosition(revision)
    if bisect_utils.IsStringInt(commit_position):
              int(commit_position) - 1, 'v8_bleeding_edge', DEPOT_DEPS_NAME, -1,

      deps_file = bisect_utils.FILE_DEPS_GIT
      if not os.path.exists(deps_file):
        deps_file = bisect_utils.FILE_DEPS
      execfile(deps_file, {}, deps_data)
              warning_text = ('Could not parse revision for %s while bisecting '
      deps_file_contents = ReadStringFromFile(deps_file)
    Args:
      depot: A depot name. Should be in the DEPOT_NAMES list.

      A dict in the format {depot: revision} if successful, otherwise None.
  def BackupOrRestoreOutputDirectory(self, restore=False, build_type='Release'):
      # Get commit position for the given SHA.
      commit_position = self.source_control.GetCommitPosition(revision)
      if commit_position:
            commit_position, self.opts.target_platform, target_arch, patch_sha)
      self.BackupOrRestoreOutputDirectory(restore=False)
      self.BackupOrRestoreOutputDirectory(restore=True)
  def PostBuildRequestAndWait(self, git_revision, fetch_build, patch=None):
    """POSTs the build request job to the try server instance.
      git_revision: A Git hash revision.
    # Create a unique ID for each build request posted to try server builders.
    # This ID is added to "Reason" property of the build.
        '%s-%s-%s' % (git_revision, patch, time.time()))
    # Always use Git hash to post build request since Commit positions are
    # not supported by builders to build.
        'revision': 'src@%s' % git_revision,
    if request_build.PostTryJob(builder_host, builder_port, job_args):
        print '%s [revision: %s]' % (error_msg, git_revision)
    print 'Failed to post build request for revision: [%s]' % git_revision
    """Checks if build can be downloaded based on target platform and depot."""
  def UpdateDepsContents(self, deps_contents, depot, git_revision, deps_key):
    """Returns modified version of DEPS file contents.

    Args:
      deps_contents: DEPS file content.
      depot: Current depot being bisected.
      git_revision: A git hash to be updated in DEPS.
      deps_key: Key in vars section of DEPS file to be searched.

    Returns:
      Updated DEPS content as string if deps key is found, otherwise None.
    """
    # Check whether the depot and revision pattern in DEPS file vars
    # e.g. for webkit the format is "webkit_revision": "12345".
    deps_revision = re.compile(r'(?<="%s": ")([0-9]+)(?=")' % deps_key,
                               re.MULTILINE)
    new_data = None
    if re.search(deps_revision, deps_contents):
      commit_position = self.source_control.GetCommitPosition(
          git_revision, self._GetDepotDirectory(depot))
      if not commit_position:
        print 'Could not determine commit position for %s' % git_revision
        return None
      # Update the revision information for the given depot
      new_data = re.sub(deps_revision, str(commit_position), deps_contents)
    else:
      # Check whether the depot and revision pattern in DEPS file vars
      # e.g. for webkit the format is "webkit_revision": "559a6d4ab7a84c539..".
      deps_revision = re.compile(
          r'(?<=["\']%s["\']: ["\'])([a-fA-F0-9]{40})(?=["\'])' % deps_key,
          re.MULTILINE)
      if re.search(deps_revision, deps_contents):
        new_data = re.sub(deps_revision, git_revision, deps_contents)
    if new_data:
      # For v8_bleeding_edge revisions change V8 branch in order
      # to fetch bleeding edge revision.
      if depot == 'v8_bleeding_edge':
        new_data = _UpdateV8Branch(new_data)
        if not new_data:
          return None
    return new_data

      updated_deps_content = self.UpdateDepsContents(
          deps_contents, depot, revision, deps_var)
      # Write changes to DEPS file
      if updated_deps_content:
        WriteStringToFile(updated_deps_content, deps_file)

    build_success = False
        build_success = True
    else:
      # These codes are executed when bisect bots builds binaries locally.
      build_success = self.builder.Build(depot, self.opts)
    # android-chrome-shell works. bisect-perf-regression.py script should
      commit_position = self.source_control.GetCommitPosition(revision,
                                                              cwd=self.src_cwd)
      if not commit_position:
      if bisect_utils.IsStringInt(commit_position) and matches:
        if commit_position <= 274857 and cmd_browser == 'android-chrome-shell':
        elif (commit_position >= 276628 and
  def _FindAllRevisionsToSync(self, revision, depot):
    """Finds all dependent revisions and depots that need to be synced.

    For example skia is broken up into 3 git mirrors over skia/src,
    skia/gyp, and skia/include. To sync skia/src properly, one has to find
    the proper revisions in skia/gyp and skia/include.
    This is only useful in the git workflow, as an SVN depot may be split into
    multiple mirrors.
    # guarantee that the SVN revision will exist for each of the dependent
      commit_position = self.source_control.GetCommitPosition(revision)
            commit_position, d, DEPOT_DEPS_NAME, -1000)
    """Performs cleanup between runs."""
    # Leaving these .pyc files around between runs may disrupt some perf tests.
      True if successful.
      True if successful.
  def _PerformPreSyncCleanup(self, depot):
    Args:
      depot: Depot name.

      # issues syncing when using the git workflow (crbug.com/377951).
  def _RunPostSync(self, depot):
    Args:
      depot: Depot name.

      if not builder.SetupAndroidBuildEnvironment(self.opts,
    """Checks whether a particular revision can be safely skipped.

    Some commits can be safely skipped (such as a DEPS roll), since the tool
  def RunTest(self, revision, depot, command, metric, skippable=False):
      command: The command to execute the performance test.
    # Decide which sync program to use.
    # Decide what depots will need to be synced to what revisions.
    revisions_to_sync = self._FindAllRevisionsToSync(revision, depot)
      return ('Failed to resolve dependent depots.', BUILD_RESULT_FAIL)
    if not self._PerformPreSyncCleanup(depot):
    # Do the syncing for all depots.
      if not self._SyncAllRevisions(revisions_to_sync, sync_client):
        return ('Failed to sync: [%s]' % str(revision), BUILD_RESULT_FAIL)

     # Try to do any post-sync steps. This may include "gclient runhooks".
    if not self._RunPostSync(depot):
      return ('Failed to run [gclient runhooks].', BUILD_RESULT_FAIL)

    # Skip this revision if it can be skipped.
    if skippable and self.ShouldSkipRevision(depot, revision):
      return ('Skipped revision: [%s]' % str(revision),
              BUILD_RESULT_SKIPPED)

    # Obtain a build for this revision. This may be done by requesting a build
    # from another builder, waiting for it and downloading it.
    start_build_time = time.time()
    build_success = self.BuildCurrentRevision(depot, revision)
    if not build_success:
      return ('Failed to build revision: [%s]' % str(revision),
              BUILD_RESULT_FAIL)
    after_build_time = time.time()
    # Possibly alter the command.
    command = self.GetCompatibleCommand(command, revision, depot)
    # Run the command and get the results.
    results = self.RunPerformanceTestAndParseResults(command, metric)

    # Restore build output directory once the tests are done, to avoid
    # any discrepancies.
    if self.IsDownloadable(depot) and revision:
      self.BackupOrRestoreOutputDirectory(restore=True)

    # A value other than 0 indicates that the test couldn't be run, and results
    # should also include an error message.
    if results[1] != 0:
      return results

    external_revisions = self._Get3rdPartyRevisions(depot)

    if not external_revisions is None:
      return (results[0], results[1], external_revisions,
          time.time() - after_build_time, after_build_time -
          start_build_time)
      return ('Failed to parse DEPS file for external revisions.',
               BUILD_RESULT_FAIL)

  def _SyncAllRevisions(self, revisions_to_sync, sync_client):
    """Syncs multiple depots to particular revisions.

    Args:
      revisions_to_sync: A list of (depot, revision) pairs to be synced.
      sync_client: Program used to sync, e.g. "gclient", "repo". Can be None.

    Returns:
      True if successful, False otherwise.
    """
    for depot, revision in revisions_to_sync:
      self.ChangeToDepotWorkingDirectory(depot)

      if sync_client:
        self.PerformPreBuildCleanup()

      # When using gclient to sync, you need to specify the depot you
      # want so that all the dependencies sync properly as well.
      # i.e. gclient sync src@<SHA1>
      if sync_client == 'gclient':
        revision = '%s@%s' % (DEPOT_DEPS_NAME[depot]['src'], revision)

      sync_success = self.source_control.SyncToRevision(revision, sync_client)
      if not sync_success:
        return False

    return True
    bad_run_results = self.RunTest(bad_rev, target_depot, cmd, metric)
      good_run_results = self.RunTest(good_rev, target_depot, cmd, metric)
  def NudgeRevisionsIfDEPSChange(self, bad_revision, good_revision,
                                 good_svn_revision=None):
      bad_revision: First known bad git revision.
      good_revision: Last known good git revision.
      good_svn_revision: Last known good svn revision.
    # DONOT perform nudge because at revision 291563 .DEPS.git was removed
    # and source contain only DEPS file for dependency changes.
    if good_svn_revision >= 291563:
      return (bad_revision, good_revision)

          FILE_DEPS, good_revision, bad_revision)
              'origin/master', '--', bisect_utils.FILE_DEPS_GIT]
      cmd = ['log', '--format=%ct', '-1', good_revision]
      # CrOS and SVN use integers.
  def CanPerformBisect(self, good_revision, bad_revision):
    Checks for following:
    1. Non-bisectable revsions for android bots (refer to crbug.com/385324).
    2. Non-bisectable revsions for Windows bots (refer to crbug.com/405274).
      good_revision: Known good revision.
      bad_revision: Known bad revision.
      revision_to_check = self.source_control.GetCommitPosition(good_revision)
      if (bisect_utils.IsStringInt(good_revision)
          and good_revision < 265549):
            'Bisect cannot continue for the given revision range.\n'

    if bisect_utils.IsWindowsHost():
      good_revision = self.source_control.GetCommitPosition(good_revision)
      bad_revision = self.source_control.GetCommitPosition(bad_revision)
      if (bisect_utils.IsStringInt(good_revision) and
          bisect_utils.IsStringInt(bad_revision)):
        if (289987 <= good_revision < 290716 or
            289987 <= bad_revision < 290716):
          return {'error': ('Oops! Revision between r289987 and r290716 are '
                            'marked as dead zone for Windows due to '
                            'crbug.com/405274. Please try another range.')}

      'depot': The depot that this revision is from (i.e. WebKit)
            'passed': False,
            'depot': 'chromium',
            'external': None,
            'sort': 0
    # If they passed SVN revisions, we can try match them to git SHA1 hashes.
      results['error'] = 'Couldn\'t resolve [%s] to SHA1.' % bad_revision_in
      results['error'] = 'Couldn\'t resolve [%s] to SHA1.' % good_revision_in
        bad_revision, good_revision, good_revision_in)
    cannot_bisect = self.CanPerformBisect(good_revision, bad_revision)
        run_results = self.RunTest(
            next_revision_id, next_revision_depot, command_to_run, metric,
            skippable=True)
      commit_info = ('\nFailed to parse SVN revision from body:\n%s' %
    if bisect_utils.IsTelemetryCommand(self.opts.command):
      telemetry_command = re.sub(r'--browser=[^\s]+',
                                 '--browser=<bot-name>',
                                 command)
      print REPRO_STEPS_TRYJOB_TELEMETRY % {'command': telemetry_command}
    else:
      print REPRO_STEPS_TRYJOB % {'command': command}
             'range of revisions where a performance metric regressed.\n')
          raise RuntimeError('Must specify try server host name using '
                             '--builder_host when gs_bucket is used.')
          raise RuntimeError('Must specify try server port number using '
                             '--builder_port when gs_bucket is used.')
      if opts.bisect_mode != BISECT_MODE_RETURN_CODE:
        metric_values = opts.metric.split('/')
        if len(metric_values) != 2:
          raise RuntimeError('Invalid metric specified: [%s]' % opts.metric)
        opts.metric = metric_values
    """Creates an instance of BisectOptions from a dictionary.
    if opts.metric and opts.bisect_mode != BISECT_MODE_RETURN_CODE:
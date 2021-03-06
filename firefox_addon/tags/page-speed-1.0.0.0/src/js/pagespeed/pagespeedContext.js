/**
 * Copyright 2007-2009 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @fileoverview Integrates Page Speed with Firebug.
 *
 * To create a new lint rule, create a new LintRule instance in a separate .js
 * file, then register the function with PAGESPEED.LintRules.registerLintRule()
 *
 * @author Kyle Scholz
 */

(function() {  // Begin closure

/**
 * PAGESPEED.PageSpeedContext is a Singleton within each window.
 * A window may contain many tabs, and there is a single PageSpeedContext
 * for all tabs in a window.
 * @constructor
 */
PAGESPEED.PageSpeedContext = function() {
  this.callbacks = {
    // Run when firebug calls PageSpeedModule.initialize().
    initPageSpeedModule: new PAGESPEED.CallbackHolder(),

    // Run after PAGESPEED.PageSpeedContext.displayPerformance() is complete.
    postDisplay: new PAGESPEED.CallbackHolder(),

    // Run after Firebug.PageSpeedModule.watchWindow() is complete on
    // the top level window object (as opposed to a frame or iframe).
    watchWindow: new PAGESPEED.CallbackHolder(),

    // Run after Firebug.PageSpeedModule.unwatchWindow() is complete on the top
    // level window object.
    unwatchWindow: new PAGESPEED.CallbackHolder(),

    // Run when the pagespeed tab is shown in firebug.
    showPageSpeed: new PAGESPEED.CallbackHolder(),

    // Run right before we invoke the rules to generate a score. All
    // callees must invoke the 'callback' property of the callback
    // argument once they are done performing any operations that need
    // to run before the scoring code runs. Failure to invoke the
    // callback will cause Page Speed to hang, since it waits until all
    // callbacks have been invoked before proceeding.
    beforeScoring: new PAGESPEED.CallbackHolder(),

    // Called when PageSpeed is considering refetching a url.  If ANY
    // callback installed returns true, then the URL will NOT be fetched.
    isRefetchForbidden: new PAGESPEED.CallbackHolder()
  };
};

/**
 * Displays the score card.
 * @param {Object} panel Firebug-created object that stores state of the
 *     the panel where results are displayed.
 * @param {Object} browserTab The browser object of the tab PageSpeed is
 *     running on.
 */
PAGESPEED.PageSpeedContext.prototype.displayPerformance = function(
    panel, browserTab) {

  // The rules array holds the rule objects used to fill the domplate.
  var rules = [];
  var totalScore = 0;
  var totalWeight = 0;
  var lintRules = PAGESPEED.LintRules.lintRules;

  /**
   * Sorts rules by their name.
   */
  var sortByName = function(a, b) {
    if (a.name == b.name) {
      return 0;
    }
    return a.name > b.name ? 1 : -1;
  };

  /**
   * Sorts rules by their color code and/or score.
   */
  var sortByScore = function(a, b) {
    var aColorCode = PAGESPEED.Utils.getColorCode(a);
    var bColorCode = PAGESPEED.Utils.getColorCode(b);
    if (aColorCode != bColorCode) {
      return aColorCode - bColorCode;
    }

    // Past this point, the color codes are the same.

    var aDetails = !!((a.warnings || '') + (a.information || ''));
    var bDetails = !!((b.warnings || '') + (b.information || ''));
    if (aDetails != bDetails) {
      return aDetails ? -1 : 1;
    }

    if (aColorCode == 4) {
      // Non-scoring (informational) color code. sort by score if both
      // scores are strings.
      var aIsString = (typeof a.score == 'string');
      var bIsString = (typeof b.score == 'string');
      if (aIsString && bIsString) {
        // If both are strings, then perform a lexical sort.
        if (a.score == b.score) {
          return sortByName(a, b);
        }
        return (a.score > b.score) ? 1 : -1;
      }
      return aIsString ? 1 : -1;
    }

    if (b.weight == a.weight) {
      return sortByName(a, b);
    }

    return b.weight - a.weight;
  };

  lintRules.sort(sortByScore);

  // Keep track of the number of each score code that we encounter.
  var scoreCodeCounts = {};
  scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_RED] = 0;
  scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_YELLOW] = 0;
  scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_GREEN] = 0;
  scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_INFO] = 0;

  for (var i = 0, len = lintRules.length; i < len; i++) {
    // Always display the rule if it is a scoring rule. If its a non-scoring
    // rule display it only if it detected a violation.
    if (lintRules[i].weight ||
        (!lintRules[i].weight &&
          lintRules[i].score != 'n/a' &&
          lintRules[i].score != 100)) {
      // If its a non-scoring rule, clear the score so it doesn't display.
      if (lintRules[i].weight === 0) {
        lintRules[i].score = 'unscored';
      }
      var colorCode = PAGESPEED.Utils.getColorCode(lintRules[i]);
      if (colorCode == PAGESPEED.Utils.SCORE_CODE_INFO &&
          !lintRules[i].information) {
        lintRules[i].information =
            'This suggestion does not apply to the current page.';
      }
      scoreCodeCounts[colorCode]++;
      rules.push(panel.createRuleTagDomplateData(lintRules[i]));
      if (!isNaN(lintRules[i].score) &&
          !isNaN(lintRules[i].weight)) {
        totalScore += lintRules[i].score * lintRules[i].weight;
        totalWeight += lintRules[i].weight;
      }
    }
  }

  var overallScore = Math.round(totalWeight ? totalScore / totalWeight : 0);

  var SCORE_CODE_THRESHOLD_ = 4;

  // Determine an overall color code based on the score code counts.
  var overallScoreColorCode = PAGESPEED.Utils.SCORE_CODE_INFO;
  if (scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_RED] >=
      SCORE_CODE_THRESHOLD_) {
    // There are enough 'red' codes that we give an overall score of
    // 'red'.
    overallScoreColorCode = PAGESPEED.Utils.SCORE_CODE_RED;
  } else if (scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_RED] > 0 ||
             scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_YELLOW] >=
                 SCORE_CODE_THRESHOLD_) {
    // There are enough 'red' or 'yellow' codes that we give an
    // overall score of 'yellow'.
    overallScoreColorCode = PAGESPEED.Utils.SCORE_CODE_YELLOW;
  } else if (scoreCodeCounts[PAGESPEED.Utils.SCORE_CODE_GREEN] > 0) {
    // If at least one rule scores green, give an overall score of
    // green.
    overallScoreColorCode = PAGESPEED.Utils.SCORE_CODE_GREEN;
  }

  panel.table = panel.tableTag.replace(
      panel.createTableTagDomplateData(overallScoreColorCode),
      panel.panelNode, panel);

  var tbody = panel.table.firstChild;
  var lastRow = tbody.lastChild;
  var row = panel.ruleTag.insertRows({'rules': rules}, lastRow)[0];
  for (var i = 0; i < rules.length; ++i) {
    row.repObject = rules[i];
    rules[i].row = row;
    row = row.nextSibling;
  }

  // Pass the browser object that holds data for the current tab.
  // There is only one callback holder per window (which can contain
  // many tabs).  Passing in a reference to the tab allows tab
  // specific information (such as the page load time) to be found.
  this.callbacks.postDisplay.execCallbacks(
      {
        // TODO: we do not expose an overall score in the UI, so we
        // should rethink the data that we send in this callback.
        overallScore: overallScore,
        browserTab: browserTab
      });
};

/**
 * Displays the list of components for the current window.
 */
PAGESPEED.PageSpeedContext.prototype.displayComponents = function(panel) {
  var comps = [];
  var totalComponents = 0;
  var totalFileSize = 0;
  var totalTransferSize = 0;
  var urls = PAGESPEED.Utils.getResources();

  for (var i = 0, len = urls.length; i < len; ++i) {
    var fileSize = PAGESPEED.Utils.getResourceSize(urls[i]);
    var transferSize = PAGESPEED.Utils.getResourceTransferSize(urls[i]);
    ++totalComponents;
    totalFileSize += fileSize;
    totalTransferSize += transferSize;

    comps.push({
                 'type': PAGESPEED.Utils.getResourceTypes(urls[i]).join(','),
                 'href': urls[i],
                 'path': PAGESPEED.Utils.getDisplayUrl(urls[i]),
                 'statusCode': PAGESPEED.Utils.getResponseCode(urls[i]),
                 'domain': PAGESPEED.Utils.getDomainFromUrl(urls[i]),
                 'filesize': PAGESPEED.Utils.formatBytes(fileSize),
                 'xfersize': PAGESPEED.Utils.formatBytes(transferSize)
    });
  }

  panel.table = panel.componentsTag.replace({
      'totalComponents': totalComponents + ' resources',
      'totalTransferSize': PAGESPEED.Utils.formatBytes(totalTransferSize),
      'totalFileSize': PAGESPEED.Utils.formatBytes(totalFileSize)},
      panel.panelNode, panel);

  var tbody = panel.table.firstChild;
  var lastRow = tbody.lastChild.previousSibling;
  var row = panel.componentTag.insertRows({'components': comps},
                                          lastRow)[0];

  // TODO: This weird boiler plate code for domplate was copied from
  // Firebug and is duplicated in displayPerformance. We should determine if
  // it can be simplified and at minimum factor out the duplication.
  for (var i = 0; i < comps.length; ++i) {
    row.repObject = comps[i];
    comps[i].row = row;
    row = row.nextSibling;
  }
};

/**
 * Helper that calls the function with name 'name' on each rule,
 * passing args as the set of args to that function. Used to call
 * initContext and destroyContext on rules, to allow them to hook
 * into these (and possibly other) firebug events.
 */
PAGESPEED.PageSpeedContext.prototype.dispatchToRules = function(name, args) {
  var lintRules = PAGESPEED.LintRules.lintRules;
  var rule;
  for (var i = 0, len = lintRules.length; i < len; ++i) {
    rule = lintRules[i];
    if (name in rule) {
      rule[name].apply(rule, args);
    }
  }
};


// Set up PageSpeedContext as a Singleton
PAGESPEED.PageSpeedContext = new PAGESPEED.PageSpeedContext();


/**
 * If Page Speed is unhealthy, stop initializing here.
 */
PAGESPEED.isHealthy && PAGESPEED.Utils.checkNamespaceDependencies();
if (!PAGESPEED.isHealthy) return;

FBL.ns(function() { with (FBL) {
/**
 * Create a Firebug Module so that we can use Firebug Hooks.
 * @extends {Firebug.Module}
 */
Firebug.PageSpeedModule = extend(Firebug.Module, {
  /**
   * Called when the window is opened.
   * @override
   */
  initialize: function() {
    PAGESPEED.PageSpeedContext.callbacks.initPageSpeedModule.execCallbacks(
        {PageSpeedModule: this});
  },

  /**
   * Called when a new context is created but before the page is loaded.
   * @override
   */
  initContext: function(context) {
    // Invoke 'initContext' on any rules that implement that method.
    PAGESPEED.PageSpeedContext.dispatchToRules('initContext', [context]);
  },

  /**
   * Called when a context is destroyed.
   * @override
   */
  destroyContext: function(context) {
    // Invoke 'destroyContext' on any rules that implement that method.
    PAGESPEED.PageSpeedContext.dispatchToRules('destroyContext', [context]);
  },

  /** @override */
  showPanel: function(browser, panel) {
    var isPageSpeed = panel && 'pagespeed' == panel.name;
    var pagespeedButtons = browser.chrome.$('fbPageSpeedButtons');
    FBL.collapse(pagespeedButtons, !isPageSpeed);

    if (isPageSpeed) {
      PAGESPEED.PageSpeedContext.callbacks.showPageSpeed.execCallbacks({});
    }
  },

  showPerformance: function() {
    PAGESPEED.LintRules.stop();
    try {
      // Get the object that represents the current tab.
      var browserOfCurrentTab = gBrowser.selectedBrowser;

      // Keep track of the number of outstanding callbacks. Once the
      // number goes to zero, we can start the scoring process.
      var numBeforeScoringCallbacks =
          PAGESPEED.PageSpeedContext.callbacks.beforeScoring.getNumCallbacks();

      if (numBeforeScoringCallbacks > 0) {
        var clientCallback = function() {
          numBeforeScoringCallbacks--;
          if (numBeforeScoringCallbacks == 0) {
            // All of the before scoring clients are finished, so we
            // can now start the scoring process.
            PAGESPEED.LintRules.exec(browserOfCurrentTab);
          }
        };

        // Tell each client that we're about to start scoring.
        PAGESPEED.PageSpeedContext.callbacks.beforeScoring.execCallbacks(
          {
            callback: clientCallback
          });
      } else {
        // No before scoring callbacks, so we can just invoke the
        // rules directly.
        PAGESPEED.LintRules.exec(browserOfCurrentTab);
      }
    } catch (e) {
      PS_LOG('Unexpected exception in showPerformance.  Please file a bug ' +
            'with the following text: ' + e + '\n');
    }
  },

  showComponents: function() {
    PAGESPEED.LintRules.stop();
    try {
      PAGESPEED.PageSpeedContext.displayComponents(
          FirebugContext.getPanel('pagespeed'));
    } catch (e) {
      PS_LOG('Unexpected exception in showComponents().  Please file a bug' +
             'with the following text: ' + e + '\n');
    }
  },

  pagespeedOnload: function(win) {
    // Record a timestamp at onload.  A timestamp is also added to each resource
    // when it is first fetched in the component collector.  This allows rules
    // to figure out which resources are loaded before onload.
    win.onloadTime = (new Date()).getTime();

    // If the autorun pref is set, then run Page Speed rules.
    if (PAGESPEED.Utils.getBoolPref('extensions.PageSpeed.autorun')) {
      var doc = win.document;
      if (!doc || !doc.location || typeof(doc.location.hostname) == 'undefined')
        return;

      // Want any javascript in the page which runs at onload to run before
      // Page Speed starts.  To allow this, launch pagespeed after a delay.  The
      // default is 100ms.  Users might want to set a longer delay to be sure
      // all javascript has run.  This can be done with the preference
      // extensions.PageSpeed.autorun.delay.
      var delay = PAGESPEED.Utils.getIntPref(
          'extensions.PageSpeed.autorun.delay', 100);
      win.setTimeout(Firebug.PageSpeedModule.showPerformance, delay);
    }
  },

  /**
   * When a window is loaded, track redirects and add an event listener for
   * page loads.
   *
   * @param {Object} context The Firebug context linked to the window.
   * @param {Object} win The window object assocated with the tab/window
   *     being loaded.
   */
  watchWindow: function(context, win) {
    // watchWindow is called on iframes as well as top level windows.  Only run
    // Page Speed when the onload event for the top level window fires.
    if (win == win.top) {
      PAGESPEED.LintRules.stop();
      PAGESPEED.PageSpeedContext.callbacks.watchWindow.execCallbacks(
        {
          window: win,
          context: context
        });

      var onloadFn = function() {
        Firebug.PageSpeedModule.pagespeedOnload(win);
      };
      win.addEventListener('load', onloadFn, false);
    }
  },

  /**
   * When a window is unloaded, remove the pagespeed object.
   * @param {Object} context The Firebug context linked to the window.
   * @param {Object} win The window object assocated with the tab/window
   *     being loaded.
   */
  unwatchWindow: function(context, win) {
    if (win == win.top) {
      PAGESPEED.PageSpeedContext.callbacks.unwatchWindow.execCallbacks(
          {
            window: win
          });
    }
  }
});  // PageSpeedModule

Firebug.registerModule(Firebug.PageSpeedModule);

}});  // FBL.ns

})();  // End closure

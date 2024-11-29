#Licensed under the MIT License.
#For details on the licensing terms, see the LICENSE file.
#SPDX-License-Identifier: MIT

#Copyright 2023-2024 (c) Fraunhofer IOSB (Author: Florian Düwel)

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'SWAP-IT Registry Module'
copyright = '2024, Fraunhofer IOSB (Author:Florian Düwel)'
author = 'Florian Düwel'
release = '1.0.0'

html_theme_options = {
    "collapse_navigation": True,
    "navigation_depth": 2
}

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['sphinx.ext.intersphinx',
              'sphinx.ext.autosectionlabel']

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

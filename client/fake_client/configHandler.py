import json
import os

def loadConfigData(relativePath = "config.json"):
  """Loads the config file as a dictionary"""
  with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), relativePath)) as file:
    config = json.load(file)
  return config
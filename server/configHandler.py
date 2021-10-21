import json
import os

def loadConfigData():
  """Loads the config file as a dictionary"""
  with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'config.json')) as file:
    config = json.load(file)
  return config
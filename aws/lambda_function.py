import json
import pyspiceql
import os

def lambda_handler(event, context):
    try: 
        print("Lambda Event: ", event)
        print("Lambda Context: ", context)
        func = getattr(pyspiceql, event["func"])
        event.pop('func')
        args = list(event.values())
        # removes empty arguments from list
        args = [arg for arg in args if arg]
        ret = func(*args)
        print("Returned: ", ret)
        return {
            "statusCode" : 200,
            "body" : json.dumps({
                "return" : ret
            })
        }
        
    except Exception as e: 
        return {
            "statusCode" : 500,
            "body" : json.dumps({
                "error" : str(e)
            })
        }
    

def create_lambda_call(func):
    def lambda_wrapper(event, context):
      try: 
        print("Lambda Event: ", event)
        print("Lambda Context: ", context)
        ret = func(**event)
        print("Returned: ", ret)
        return {
            "statusCode" : 200,
            "body" : json.dumps({
                "return" : ret
            })
        }
        
      except Exception as e: 
        return {
            "statusCode" : 500,
            "body" : json.dumps({
                "error" : str(e)
            })
        }
    return lambda_wrapper

lambda_strSclkToEt = create_lambda_call(pyspiceql.strSclkToEt)
lambda_doubleSclkToEt = create_lambda_call(pyspiceql.doubleSclkToEt)
lambda_utcToEt = create_lambda_call(pyspiceql.utcToEt)
lambda_translateNameToCode = create_lambda_call(pyspiceql.Memo_translateNameToCode)
lambda_translateCodeToName = create_lambda_call(pyspiceql.Memo_translateCodeToName)
lambda_getTargetStates = create_lambda_call(pyspiceql.getTargetStates)
lambda_getFrameInfo = create_lambda_call(pyspiceql.getFrameInfo)
lambda_findMissionKeywords = create_lambda_call(pyspiceql.findMissionKeywords)
lambda_getTargetValues = create_lambda_call(pyspiceql.getTargetValues)

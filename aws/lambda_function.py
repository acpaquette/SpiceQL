import pyspiceql

def lambda_handler(event, context):
    try: 
        print("Lambda Event: ", event)
        print("Lambda Context: ", context)
        func = getattr(pyspiceql, event["func"])
        event.pop('func')
        args = list(event.values())
        # removes empty arguments from list
        args = [arg for arg in args if arg]
        ret = {"return": func(*args)}
        print(ret)
        return {
            "statusCode" : 200,
            "body" : ret
        }

    except Exception as e:
        return {
            "statusCode" : 500,
            "body" : {
                "error" : str(e)
            }
        }
    

def create_lambda_call(func):
    def lambda_wrapper(event, context):
      try: 
        print("Lambda Event: ", event)
        print("Lambda Context: ", context)
        ret = {"return": func(**event)}
        print(ret)
        return {
            "statusCode" : 200,
            "body" : ret
        }
        
      except Exception as e: 
        return {
            "statusCode" : 500,
            "body" : {
                "error" : str(e)
            }
        }
    return lambda_wrapper

lambda_strSclkToEt = create_lambda_call(pyspiceql.strSclkToEt)
lambda_doubleSclkToEt = create_lambda_call(pyspiceql.doubleSclkToEt)
lambda_utcToEt = create_lambda_call(pyspiceql.utcToEt)
lambda_translateNameToCode = create_lambda_call(pyspiceql.translateNameToCode)
lambda_translateCodeToName = create_lambda_call(pyspiceql.translateCodeToName)
lambda_getTargetStates = create_lambda_call(pyspiceql.getTargetStates)
lambda_getTargetOrientations = create_lambda_call(pyspiceql.getTargetOrientations)
lambda_getFrameInfo = create_lambda_call(pyspiceql.getFrameInfo)
lambda_findMissionKeywords = create_lambda_call(pyspiceql.findMissionKeywords)
lambda_findTargetKeywords = create_lambda_call(pyspiceql.findTargetKeywords)
lambda_getTargetFrameInfo = create_lambda_call(pyspiceql.getTargetFrameInfo)
lambda_frameTrace = create_lambda_call(pyspiceql.frameTrace)
lambda_extractExactCkTimes = create_lambda_call(pyspiceql.extractExactCkTimes)


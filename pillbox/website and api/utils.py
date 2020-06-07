import boto3
from cred import *
from werkzeug.security import generate_password_hash, check_password_hash

TOPIC_PREFIX = 'arn:aws:sns:us-west-2:256987428706:'
SENDER_ID = 'PillBox'

sns = boto3.client('sns', aws_access_key_id = AWS_ID, aws_secret_access_key = AWS_SECRET_KEY, region_name='us-west-2')

def deCanonize(phone_number):
	return '0' + phone_number[4:]

def Canonize(phone_number):
	return '+972' + phone_number[1:]


def getTopicSubscribers(topic):
	new_topic = TOPIC_PREFIX + topic
	response = sns.list_subscriptions_by_topic(TopicArn=new_topic)
	subscriptions = []
	for subscription in response['Subscriptions']:
		subscriptions.append(subscription['Endpoint'])
	return subscriptions


def getAllTopics():
	response = sns.list_topics()
	topics = [topic['TopicArn'].split(':') for topic in response['Topics']]
	topics = [topic[-1] for topic in topics]
	return topics

def addTopic(topic):
	current_topics = getAllTopics()
	if topic in current_topics:
		return -1
	sns.create_topic(Name=topic)
	return 0

def addSubscriber(topic, subscriber):
	topic_arn = TOPIC_PREFIX + topic
	current_subs = getTopicSubscribers(topic)
	if Canonize(subscriber) not in current_subs:
		print("Adding " + str(Canonize(subscriber)))
		sns.subscribe(TopicArn=topic_arn, Protocol='sms', Endpoint=Canonize(subscriber))
	else:
		print("Subscriber " + subscriber + " Already Registered To Topic")

def removeSubscriber(topic, subscriber):
	topic_arn = TOPIC_PREFIX + topic
	response = sns.list_subscriptions_by_topic(TopicArn=topic_arn)
	for subscription in response['Subscriptions']:
		if subscription['Endpoint'] == Canonize(subscriber):
			sns.unsubscribe(SubscriptionArn=subscription['SubscriptionArn'])


def publishTopic(topic, message, sender=None):
	senderId = SENDER_ID if sender is None else sender
	sns.set_sms_attributes(attributes={"DefaultSenderID": senderId})
	topic_arn = TOPIC_PREFIX + topic
	sns.publish(Message=message, TopicArn=topic_arn)


def hash(password):
	return generate_password_hash(password)


def checkPass(hash, password):
	return check_password_hash(hash, password)

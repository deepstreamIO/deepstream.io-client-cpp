/*
 * Copyright 2017 deepstreamHub GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DEEPSTREAM_EVENT_HPP
#define DEEPSTREAM_EVENT_HPP

#include <functional>
#include <map>
#include <memory>
#include <vector>


namespace deepstream
{
	struct Buffer;
	struct Message;


	struct Event
	{
		typedef Buffer Name;

		/**
		 * This alias is the signature of a deepstream event subscription
		 * callback.
		 */
		typedef std::function<void(const Buffer&)> SubscribeFn;
		/**
		 * The representation of a callback is stored as a smart pointer.
		 *
		 * Given an event name, the deepstream API allows the selective removal
		 * of subscription callbacks. Hence, we need to be able to compare
		 * functions. This is not possible `std::function` objects but with a
		 * smart pointer we solve two problems:
		 * - we can compare function references for equality,
		 * - if a callback removes itself as a callback, we can ensure that the
		 *   `std::function` destructor is not called before returning.
		 */
		typedef std::shared_ptr<SubscribeFn> SubscribeFnPtr;
		typedef std::vector<SubscribeFnPtr> SubscriberList;
		typedef std::map<Name, SubscriberList> SubscriberMap;

		/**
		 * The following alias is the signature of a deepstream event listener
		 * callback.
		 */
		typedef std::function<bool(const Name&, bool, const Name&)> ListenFn;
		/**
		 * The representation of a callback is stored as a smart pointer.
		 */
		typedef std::shared_ptr<ListenFn> ListenFnPtr;
		typedef std::map<Name, ListenFnPtr> ListenerMap;

		/**
		 * This alias is the signature of the function used to send messages to
		 * the deepstream server.
		 *
		 * Function matching this signature are expected to return `true` if the
		 * message was sent successfully, `false` otherwise.
		 */
		typedef std::function<bool(const Message&)> SendFn;


		/**
		 * The constructor takes a function that sends a message to the
		 * deepstream server.
		 *
		 * With this constructor instead of `Event(deepstream::Client*)` it
		 * becomes easier to test this module.
		 */
		explicit Event(const SendFn&);


		/**
		 * This function emits an event with the provided name and the given
		 * buffer as its data.
		 */
		void emit(const Name&, const Buffer&);

		/**
		 * This method subscribes the given function to the event with the given
		 * name.
		 *
		 * @return A smart pointer which can be used to unsubscribe
		 */
		SubscribeFnPtr subscribe(const Name&, const SubscribeFn&);
		void subscribe(const Name&, const SubscribeFnPtr&);

		/**
		 * This function unsubscribes *all* callbacks from the given event.
		 */
		void unsubscribe(const Name&);

		/**
		 * This function removes the given callback from the list of subscribers
		 * for the given event.
		 */
		void unsubscribe(const Name&, const SubscribeFnPtr&);

		/**
		 * This method adds a callback that listens for subscriptions matching
		 * the given pattern.
		 *
		 * Existing listeners are not overwritten.
		 */
		ListenFnPtr listen(const Name&, const ListenFn&);
		void listen(const Name& pattern, const ListenFnPtr&);
		void unlisten(const Name& pattern);


		/**
		 * This method is called when event-related messages arrive (messages
		 * with topic `Topic::EVENT`) from the server.
		 */
		void notify_(const Message&);
		/**
		 * This method handles messages from the server related to event
		 * subscription.
		 */
		void notify_subscribers_(const Message&);
		/**
		 * This method handles messages from the server related to event
		 * listening.
		 */
		void notify_listeners_(const Message&);


		SendFn send_;
		SubscriberMap subscriber_map_;
		ListenerMap listener_map_;
	};
}

#endif

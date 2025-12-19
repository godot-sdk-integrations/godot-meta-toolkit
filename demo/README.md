# Godot Meta Toolkit Demo

> Note: If using Godot 4.5 or earlier, you'll need to add the Manage External Storage permission
> in export settings for downloadable content to work.

This is a demo project demonstrating usage of the Meta Platform SDK in a Godot project. Features of the Platform SDK showcased include:

- Achievements (simple, count, and bitfield)
- In-app purchases / Downloadable content
- Displaying bidirectional followers that also own the app
- Displaying user's name, profile image, and entitlement status

For more info on using the Platform SDK, see [Getting Started with the Meta Platform SDK](https://godot-sdk-integrations.github.io/godot-meta-toolkit/manual/platform_sdk/getting_started.html) in the official docs.

## Setup Info

If you wish to test this demo for yourself, you will have to submit your own build of the app on the
[Quest Developer Dashboard](https://developers.meta.com/horizon/manage). The features of this app
require the following Data Use Checkup permissions:

- User ID
- User profile
- In-app purchases and/or downloadable content
- Subscriptions
- Friends

In order for the achievements and IAP examples to work, you will have to upload these yourself as well.

### Achievements

To test the achievements used in the demo, add the following achievements in the Developer Dashboard at Engagement -> Achievements.

- An achievement of type `Simple` with the API name `simple-achievement-example`.
- An achievement of type `Count` with the API name `count-achievement-example` and a target value of `3`.
- An achievement of type `Bitfield` with the API name `bitfield-achievement-example`, a target value of `3`, and a bitfield length of `5`.

### In-App Purchases

To test the in-app purchases used in the demo, add the following add-ons in the Developer Dashboard at Monetization -> Add-ons

- A `Durable` type add-on with SKU `0001`. The demo's `durable_addon.tscn` should be exported as a PCK
and uploaded as a DLC file for this add-on. For more info on how to do this, see the documentation for
[Downloadable Content](https://godot-sdk-integrations.github.io/godot-meta-toolkit/manual/platform_sdk/downloadable_content.html).
- A `Consumable` type add-on with SKU `0002`.

A subscription with SKU `0003` should also be created at Monetization -> Subscriptions.
